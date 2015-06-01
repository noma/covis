// Copyright (c) 2015 Tobias Kramer <tobias.kramer@mytum.de>
// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include <iostream>
#include <string>
#include "ComputeConfig.h"
#include "BodyParticleSystem.h"
#include "ConfigParser.h"

int main( int argc, char **argv )
{
	std::string configFilename = "config.cfg";

	if (argc >= 2)
	{
		configFilename = argv[1];
	}
	ConfigParser cfgParser(configFilename);
	ComputeConfig config(cfgParser);

	BodyParticleSystem cometDust(config);
	cometDust.RunSimulation();

	return 0;
}
