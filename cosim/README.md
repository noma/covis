# Terms of use

Please see the [LICENSE](LICENSE) file. If in doubt please contact the authors.

# Building

## Dependencies

Install the thirdparty modules to load polyhedral comet models in
[wavefront OBJ format][http://en.wikipedia.org/wiki/Wavefront_.obj_file].

Build [tinyobjloader][https://github.com/syoyo/tinyobjloader]:

	```
	cd thirdparty/tinyobjloader
	mkdir -p build
	cd build
	cmake -DCMAKE_INSTALL_PREFIX=../../ ..
	make install
	```

## Project

Build cosim and oclinfo:

	```
	mkdir -p build
	cd build
	cmake ..
	make
	```

# Running

## Configuration

Make a copy of config.cfg.default and adjust parameters for 
1. the OpenCL device setup
2. cometary nucleus parameters

You can use the oclinfo uitility to get a list of platform and device IDs on
the computer. 

configuration string      | value explanation
--------------------------|-----------------
OPENCL_PLATFORM_ID        | OpenCL Platform ID
OPENCL_DEVICE_ID          | OpenCL Device ID
OUTPUT_STEPS              | write file output every OUTPUT_STEPS steps
COMET_OBJ_FILE            | polyhedral shape file file (OBJ format) of the comet, must be a pure triangle mesh
COMET_DENSITY             | uniform comet density in kg/m^3
COMET_ANGULAR_FREQUENCY   | 2*pi/(rotation period in seconds)
PARTICLE_COUNT            | number of trajectories to compute
PARTICLE_INITIAL_VELOCITY | v_init in m/s
PARTICLE_INITIAL_HEIGHT   | h_init in m
DELTA_T                   | integration time-step in s

## Executing the program

Run cosim with a config file specified as the first command line argument. If
no argument is given, "config.cfg" is tried. We assume cosim to be the working
directory:

	```
	build/cosim [config_file]
	```

Computations involving 20,000 triangular faces and 20,0000 particles require
modern accelerator (GPGPU or Xeon Phi) hardware with high double precision
floating point performance.

Reduce the number of particles (config PARTICLE_COUNT) first to obtain samples
of trajectories and check the results.

## Benchmark

For comparable values, we added a benchmark.cfg for generating comparable 
performance results. The current OpenCL kernel is not yet optimised.

	```
	build/cosim benchmark.cfg
	```

## Generated Output Data

Every OUTPUT_STEPS steps (i.e. after OUTPUT_STEPS*DELTA_T seconds of
simulation) all particle positions and velocities are stored in a file
pNNNNN where NNNNNN denotes the number of steps. Files are created within a
newly created directory colocated with and named after the config file.

Each line number matches to the triangle index in the OBJ file and 
contains 3 positions in cols 1-3 col 4: 0.0 col 5-7 velocities and in
col 8 0.000000 if the particle is propagated and 1.000000 if the particle
re-collided with the surface. 

# Analysis

The obtained data can be visualised with the covis programme.

