// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "ComputeConfig.h"

#include <iostream>
#include <sstream>
#include <fstream>

ComputeConfig::ComputeConfig(ConfigParser& configParser) : configParser(configParser)
{
	read(configParser);
}

const ConfigParser& ComputeConfig::getConfigParser() const
{
	return configParser;
}

void ComputeConfig::read(ConfigParser& configParser)
{
	opencl_platform_id = configParser.getIntKeyValue("OPENCL_PLATFORM_ID");
	opencl_device_id = configParser.getIntKeyValue("OPENCL_DEVICE_ID");
	step_count = configParser.getIntKeyValue("STEP_COUNT");
	output_step_count = configParser.getIntKeyValue("OUTPUT_STEP_COUNT");

	comet_obj_file = configParser.getStringKeyValue("COMET_OBJ_FILE");
	if (!ConfigParser::isFileValid(comet_obj_file))
	{
		std::cerr << "Input comet file not found." << std::endl;
		exit(-1);
	}

	//output_path = configParser.getStringKeyValue("OUTPUT_PATH");

	comet_density = configParser.getDoubleKeyValue("COMET_DENSITY"); // kg/m³
	comet_angular_frequency = configParser.getDoubleKeyValue("COMET_ANGULAR_FREQUENCY");
	particle_count = configParser.getIntKeyValue("PARTICLE_COUNT");
	particle_initial_velocity = configParser.getDoubleKeyValue("PARTICLE_INITIAL_VELOCITY");
	particle_initial_height = configParser.getDoubleKeyValue("PARTICLE_INITIAL_HEIGHT");
	delta_t = configParser.getDoubleKeyValue("DELTA_T");
}


void ComputeConfig::write(std::ostream& os)
{
	writeKey(os, "OPENCL_PLATFORM_ID", opencl_platform_id);
	writeKey(os, "OPENCL_DEVICE_ID", opencl_device_id);
	writeKey(os, "STEP_COUNT", step_count);
	writeKey(os, "OUTPUT_STEP_COUNT", output_step_count);

	writeKey(os, "COMET_OBJ_FILE", comet_obj_file);
	//writeKey(os, "OUTPUT_PATH", output_path);

	writeKey(os, "COMET_DENSITY", comet_density);
	writeKey(os, "COMET_ANGULAR_FREQUENCY", comet_angular_frequency);
	writeKey(os, "PARTICLE_COUNT", particle_count);
	writeKey(os, "PARTICLE_INITIAL_VELOCITY", particle_initial_velocity);
	writeKey(os, "PARTICLE_INITIAL_HEIGHT", particle_initial_height);
	writeKey(os, "DELTA_T", delta_t);
}

void ComputeConfig::write(std::string& filename)
{
	std::ofstream ofs(filename);
	ofs.precision(17);
	ofs << std::scientific;
	write(ofs);
	ofs.close();
}


