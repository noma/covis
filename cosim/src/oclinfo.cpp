// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "CL/cl.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
	// get OpenCL plaforms
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	
	const std::string ind = "  ";
	
	for (size_t i = 0; i < platforms.size(); ++i)
	{
		auto& platform = platforms[i];
		// output platform info:
		std::cout << "Platform " << i << ": " << std::endl;
			
		// output device type
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);


		for (size_t j = 0; j < devices.size(); ++j)
		{
			auto& device = devices[j];
			std::string result;
	
			// output device info
			std::cout << ind << ind << "Device " << j << ": " << std::endl;
			
			device.getInfo(CL_DEVICE_NAME, &result);
			std::cout << ind << ind << ind << "CL_DEVICE_NAME: " << result << std::endl;
			device.getInfo(CL_DEVICE_VENDOR, &result);
			
			std::cout << ind << ind << ind << "CL_DEVICE_VENDOR: " << result << std::endl;
			device.getInfo(CL_DEVICE_PROFILE, &result);
			std::cout << ind << ind << ind << "CL_DEVICE_PROFILE: " << result << std::endl;
			device.getInfo(CL_DEVICE_VERSION, &result);
			std::cout << ind << ind << ind << "CL_DEVICE_VERSION: " << result << std::endl;
			device.getInfo(CL_DRIVER_VERSION, &result);
			std::cout << ind << ind << ind << "CL_DRIVER_VERSION: " << result << std::endl;
			device.getInfo(CL_DEVICE_OPENCL_C_VERSION, &result);
			std::cout << ind << ind << ind << "CL_DEVICE_OPENCL_C_VERSION: " << result << std::endl;
			device.getInfo(CL_DEVICE_EXTENSIONS, &result);
			std::cout << ind << ind << ind << "CL_DEVICE_EXTENSIONS: " << result << std::endl;
		}
	}
	
	return 0;
}
