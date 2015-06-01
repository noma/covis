// Copyright (c) 2015 Tobias Kramer <tobias.kramer@mytum.de>
// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "BodyParticleSystem.h"

#include <CL/cl.hpp>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <sys/stat.h> // mkdir()
#include "tiny_obj_loader.h"
#include "ComputeConfig.h"

#include "integrate_eom_kernel.h" // generated kernel header

#define Real_t double

void triangle_normal(double *aIn, double *bIn, double *cIn, double *nv)
{
	double a[3],b[3],c[3];
	
	a[0]=bIn[0]-aIn[0];
	a[1]=bIn[1]-aIn[1];
	a[2]=bIn[2]-aIn[2];
	
	b[0]=cIn[0]-aIn[0];
	b[1]=cIn[1]-aIn[1];
	b[2]=cIn[2]-aIn[2];
	

	c[0] = (a[1] * b[2]) - (a[2] * b[1]);
	c[1] = (a[2] * b[0]) - (a[0] * b[2]);
	c[2] = (a[0] * b[1]) - (a[1] * b[0]);

    double norm = sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
	
	nv[0] = c[0]/norm;
	nv[1] = c[1]/norm;
	nv[2] = c[2]/norm;
}

void prepare_gravity(Real_t nv[], Real_t rij[], Real_t cm[], int numfaces, std::vector<unsigned int>& fi, std::vector<float>& ev)
{
	for(int i = 0; i < numfaces; i++)
	{
		double a[3], b[3], c[3], d[3], nu[3];

		int ain = fi[i*3+0];
		int bin = fi[i*3+1];
		int cin = fi[i*3+2];
		
		a[0] = ev[ain*3+0];
		a[1] = ev[ain*3+1];
		a[2] = ev[ain*3+2];
		
		b[0] = ev[bin*3+0];
		b[1] = ev[bin*3+1];
		b[2] = ev[bin*3+2];
		
		c[0] = ev[cin*3+0];
		c[1] = ev[cin*3+1];
		c[2] = ev[cin*3+2];
		
		d[0] = (a[0]+b[0]+c[0])/3.0;
		d[1] = (a[1]+b[1]+c[1])/3.0;
		d[2] = (a[2]+b[2]+c[2])/3.0;
		
		// face center
		cm[i*3+0] = d[0];
		cm[i*3+1] = d[1];
		cm[i*3+2] = d[2];
		
		triangle_normal(a, b, c, nu);
		
		nv[i*3+0] = nu[0];
		nv[i*3+1] = nu[1];
		nv[i*3+2] = nu[2];

		rij[(i*4+0)*3+0] = a[0];
		rij[(i*4+0)*3+1] = a[1];
		rij[(i*4+0)*3+2] = a[2];

		rij[(i*4+1)*3+0] = b[0];
		rij[(i*4+1)*3+1] = b[1];
		rij[(i*4+1)*3+2] = b[2];

		rij[(i*4+2)*3+0] = c[0];
		rij[(i*4+2)*3+1] = c[1];
		rij[(i*4+2)*3+2] = c[2];

		rij[(i*4+3)*3+0] = a[0];
		rij[(i*4+3)*3+1] = a[1];
		rij[(i*4+3)*3+2] = a[2];
	}
}

BodyParticleSystem::BodyParticleSystem(ComputeConfig& config)
	: config(config), stats(config.step_count)
{
}

BodyParticleSystem::~BodyParticleSystem()
{
	delete[] hposold;
	delete[] hvelold;
	delete[] hposnew;
	delete[] hvelnew;

	delete[] hnv;
	delete[] hcm;
	delete[] hrij;
}

void BodyParticleSystem::Initialize()
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, config.comet_obj_file.c_str());

	if (!err.empty()) {
	  std::cerr << err << std::endl;
	  exit(1);
	}

	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;

    // float reads, but all derived and used quantities are doubles
	std::vector<unsigned int> fi = shapes[0].mesh.indices; // face indices MatheMatica
	std::vector<float> ev = shapes[0].mesh.positions; // vertices MatheMatica

	NUM_VERTICES_PER_FACE = 3;
	NUM_FACES = shapes[0].mesh.indices.size() / NUM_VERTICES_PER_FACE;

	assert(NUM_VERTICES_PER_FACE == 3);
	assert(NUM_VERTICES_PER_FACE % NUM_VERTICES_PER_FACE == 0);

	if (config.particle_count <= 0)
		config.particle_count = NUM_FACES;

	hposold  = new Real_t[4*config.particle_count];
	hvelold  = new Real_t[4*config.particle_count];
	hposnew  = new Real_t[4*config.particle_count];
	hvelnew  = new Real_t[4*config.particle_count];

	hnv      = new Real_t[3*NUM_FACES];
	hcm      = new Real_t[3*NUM_FACES];
	hrij     = new Real_t[3*4*NUM_FACES];

    // hnv: normal vectors, hrij: collect 4 vertices per triangle last=copy op first vertex, hcm: center of triangle
	prepare_gravity(hnv, hrij, hcm, NUM_FACES, fi, ev);

    // initial positions and velocities
	for(int ip = 0; ip < config.particle_count; ip++)
	{
		hposold[ip*4+0] = hcm[ip*3+0]+config.particle_initial_height*hnv[ip*3+0];
		hposold[ip*4+1] = hcm[ip*3+1]+config.particle_initial_height*hnv[ip*3+1];
		hposold[ip*4+2] = hcm[ip*3+2]+config.particle_initial_height*hnv[ip*3+2];
		hposold[ip*4+3] = 0.0;

		hvelold[ip*4+0] = hnv[ip*3+0]*config.particle_initial_velocity;
		hvelold[ip*4+1] = hnv[ip*3+1]*config.particle_initial_velocity;
		hvelold[ip*4+2] = hnv[ip*3+2]*config.particle_initial_velocity;
		hvelold[ip*4+3] = 0.0;
	}
}

void BodyParticleSystem::InitializeOpenCL()
{
	// Get available platforms
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	// Select platform from config and create a context
	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[config.opencl_platform_id])(), 0 };
	context = cl::Context(CL_DEVICE_TYPE_ALL, cps);

	// Get a list of devices on this platform
	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

	// Create a command queue and use the device from config
	queue = cl::CommandQueue(context, devices[config.opencl_device_id], CL_QUEUE_PROFILING_ENABLE);

	// Read source file
//	std::ifstream sourceFile_eom("cl/integrate_eom_kernel.cl");
//	std::string sourceCode_eom(std::istreambuf_iterator<char>(sourceFile_eom),(std::istreambuf_iterator<char>()));
//	Program::Sources source_eom(1, std::make_pair(sourceCode_eom.c_str(), sourceCode_eom.length()+1));
	// NOTE: use kernel string from generated include file
	cl::Program::Sources source_eom(1, std::make_pair((const char*)integrate_eom_kernel_cl, integrate_eom_kernel_cl_len));
	// Make program of the source code in the context
	program_eom = cl::Program(context, source_eom);
	// Build program for these specific devices, compiler argument to include local directory for header files (.h)
	program_eom.build(devices, "");
	cl_int err = 0;
	std::string buildInfo = program_eom.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[config.opencl_device_id], &err);
	std::cout << "BuildInfo: " << buildInfo << std::endl;

	if (err != CL_SUCCESS)
	{
		std::cout << "OpenCL kernel build failed, exiting." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Make kernel
	kernel_eom = cl::Kernel(program_eom, "integrate_eom");
	fprintf(stderr,"building integrate_eom done\n");


	// Create memory buffers on OpenCL device and populate them with the initial data
	gposold  = cl::Buffer(context, CL_MEM_READ_WRITE, 4*config.particle_count * sizeof(Real_t));
	gvelold  = cl::Buffer(context, CL_MEM_READ_WRITE, 4*config.particle_count * sizeof(Real_t));
	gposnew  = cl::Buffer(context, CL_MEM_READ_WRITE, 4*config.particle_count * sizeof(Real_t));
	gvelnew  = cl::Buffer(context, CL_MEM_READ_WRITE, 4*config.particle_count * sizeof(Real_t));
	gnv      = cl::Buffer(context, CL_MEM_READ_ONLY,  3*NUM_FACES * sizeof(Real_t));
	grij     = cl::Buffer(context, CL_MEM_READ_ONLY,  4*3*NUM_FACES * sizeof(Real_t));

	// Set invariant kernel arguments
	kernel_eom.setArg( 4, gnv);
	kernel_eom.setArg( 5, grij);
	kernel_eom.setArg( 6, config.particle_count);
	kernel_eom.setArg( 7, NUM_FACES);
	kernel_eom.setArg( 8, NUM_VERTICES_PER_FACE );	
	kernel_eom.setArg( 9, config.delta_t);
	kernel_eom.setArg(10, config.comet_angular_frequency);
	kernel_eom.setArg(11, config.const_gravity * config.comet_density);

	// transfer initial data
	queue.enqueueWriteBuffer(gposold, CL_TRUE, 0, 4*config.particle_count * sizeof(Real_t), hposold);
	queue.enqueueWriteBuffer(gvelold, CL_TRUE, 0, 4*config.particle_count * sizeof(Real_t), hvelold);
	queue.enqueueWriteBuffer(gnv    , CL_TRUE, 0, 3*NUM_FACES * sizeof(Real_t), hnv);
	queue.enqueueWriteBuffer(grij   , CL_TRUE, 0, 4*3*NUM_FACES * sizeof(Real_t), hrij);
}

void BodyParticleSystem::PropagateStep()
{
	// double buffering scheme
	if (step_counter % 2 == 0)
	{
		kernel_eom.setArg( 0, gposold);
		kernel_eom.setArg( 1, gvelold);
		kernel_eom.setArg( 2, gposnew);
		kernel_eom.setArg( 3, gvelnew);
	}
	else
	{
		kernel_eom.setArg(0, gposnew);
		kernel_eom.setArg(1, gvelnew);
		kernel_eom.setArg(2, gposold);
		kernel_eom.setArg(3, gvelold);
	}
	// Run the kernel on specific ND range
	cl::NDRange global(config.particle_count);
	cl::Event event;

	queue.enqueueNDRangeKernel(kernel_eom, cl::NullRange, global, cl::NullRange, 0, &event);
	event.wait();

	double t_start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	double t_end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
	stats.add(static_cast<ham::util::time::rep>(t_end - t_start));
	
	++step_counter;
}

void BodyParticleSystem::WriteState(const std::string& pathPrefix, int it)
{
	// generate filename
	char filename[500];
	sprintf(filename, "%s/p%06d.dat", pathPrefix.c_str(), it);
	std::cout << "Writing to: " << filename << std::endl;

	// transfer memory back to CPU
	queue.enqueueReadBuffer(gposold, CL_TRUE, 0, 4*config.particle_count*sizeof(Real_t), hposold);
	queue.enqueueReadBuffer(gvelold, CL_TRUE, 0, 4*config.particle_count*sizeof(Real_t), hvelold);
	FILE *fd = fopen(filename,"w");
	for(int i = 0; i < config.particle_count; ++i)
	{
		fprintf(fd,"%f %f %f %f"   , hposold[i*4+0], hposold[i*4+1], hposold[i*4+2], hposold[i*4+3]);
		fprintf(fd," %f %f %f %f\n", hvelold[i*4+0], hvelold[i*4+1], hvelold[i*4+2], hvelold[i*4+3]);
	}
	fclose(fd);
}

void BodyParticleSystem::GetParticles(int NumBodies, Real_t *pos, Real_t *vel )
{
	// transfer memory from OpenCL device to CPU
	queue.enqueueReadBuffer(gposold, CL_TRUE, 0, 4*NumBodies*sizeof(Real_t), pos);
	queue.enqueueReadBuffer(gvelold, CL_TRUE, 0, 4*NumBodies*sizeof(Real_t), vel);
	queue.finish();
}

void BodyParticleSystem::PutParticles(int NumBodies, Real_t *pos, Real_t *vel )
{
	// transfer memory from CPU to OpenCL device
	queue.enqueueWriteBuffer(gposold, CL_TRUE, 0, 4*NumBodies * sizeof(Real_t), pos);
	queue.enqueueWriteBuffer(gvelold, CL_TRUE, 0, 4*NumBodies * sizeof(Real_t), vel);
	queue.finish();
}

void BodyParticleSystem::RunSimulation()
{
	Initialize();
	InitializeOpenCL();
	PutParticles(config.particle_count, hposold, hvelold);

	const std::string& configFilename = config.getConfigParser().getFilename();
	const std::string& pathPrefix = configFilename.substr(0, configFilename.find_last_of('.'));

    if (mkdir(pathPrefix.c_str(), 0755) == -1) {
	    std::cout << "Could not create directory '" << pathPrefix << "', exiting." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::cout.precision(8);
	std::cout << std::fixed;
    
	time_t c0, c1;
	time(&c0);
	int it = 0;
	WriteState(pathPrefix, it); // write initial state
	for(it = 1; it <= config.step_count; ++it) // main propagation loop
	{
		PropagateStep();
		if ((it) % config.output_step_count == 0)
		{
			WriteState(pathPrefix, it);
			// output current statistics
			auto avg_s = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(stats.average());
			std::cout << "Average OpenCL kernel runtime per iteration: " << avg_s.count() << " s" << std::endl;
		}
	}
	// NOTE: no final write, to have only equidistant simulation time intervalls between output values
	time(&c1);
	fprintf(stderr, "Simulation for %d particles over %d steps took %.0f s\n", config.particle_count, config.step_count, difftime(c1, c0));
}

