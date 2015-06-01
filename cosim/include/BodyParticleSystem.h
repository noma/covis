// Copyright (c) 2015 Tobias Kramer <tobias.kramer@mytum.de>
// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

/* 
Compute gravitational particle trajectories orginating from a body defined
by triangular mesh cells. Assumes vertices given in meters with 
- center of mass at (0,0,0) 
- rotation around axis aligned with (0,0,1)-direction 
*/

#ifndef BodyParticleSystem_h
#define BodyParticleSystem_h

#include <CL/cl.hpp>
#include <cstdlib>

#include "ComputeConfig.h"
#include "ham/util/time.hpp"

#define Real_t double

class BodyParticleSystem {

public:
	BodyParticleSystem(ComputeConfig& config);
	~BodyParticleSystem();
	void RunSimulation();

private:
	void Initialize();
	void InitializeOpenCL();
	void PropagateStep();
	void PutParticles(int NumBodies, Real_t *hposnew, Real_t *hvelnew );
	void GetParticles(int NumBodies, Real_t *hposnew, Real_t *hvelnew );
	void WriteState(const std::string& pathPrefix, int it);

	ComputeConfig& config;
	size_t step_counter = 0;
	ham::util::time::statistics stats;

	cl::Context      context;
	cl::CommandQueue queue;
	cl::Kernel       kernel_eom;
	cl::Program      program_eom;

	cl::Buffer gposold; // compute device: positions
	cl::Buffer gvelold; // compute device: velocities
	cl::Buffer gposnew; // compute device: store temp positions
	cl::Buffer gvelnew; // compute device: store temp velocities
	cl::Buffer gnv;
	cl::Buffer grij;

	Real_t *hposold;
	Real_t *hvelold;
	Real_t *hposnew;
	Real_t *hvelnew;

	Real_t *hnv;
	Real_t *hrij;
	Real_t *hcm;

	int NUM_FACES;
	int NUM_VERTICES_PER_FACE;
};

#endif // BodyParticleSystem_h 
