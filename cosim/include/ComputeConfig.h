// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef ComputeConfig_h
#define ComputeConfig_h

#include "ConfigParser.h"

#include <string>
#include <ostream>

class ComputeConfig
{
public:
	ComputeConfig(ConfigParser& configParser);
	const ConfigParser& getConfigParser() const;
	void read(ConfigParser& configParser);
	// to print to ostream
	void write(std::ostream& stream);
	void write(std::string& filename);

	int opencl_platform_id;
	int opencl_device_id;
	int step_count;
	int output_step_count;
	//std::string output_path;
	std::string comet_obj_file;
	double comet_density; // kg/m³
	double comet_angular_frequency; // 1/s
	int particle_count;
	double particle_initial_velocity; // m/s
	double particle_initial_height; // m
	double delta_t; // s
	
	const double const_gravity = 6.67384E-11;
	const double const_pi = 3.1415926535897932385;
	
private:
	const ConfigParser& configParser;

	template<typename T>
	void writeKey(std::ostream& os, std::string key, T value)
	{
		os << key << "=" << value << std::endl;
	}
};

#endif // ComputeConfig_h
