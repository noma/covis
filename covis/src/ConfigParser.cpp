// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "ConfigParser.h"
#include <iostream>
#include <sstream>

ConfigParser::ConfigParser(const std::string& filename) : valid(true), configFile(filename.c_str()), filename(filename)
{
	if (!configFile.is_open())
	{
		valid = false;
		// debug output
		std::cout << "ConfigParser::ConfigParser(const std::string& filename): Warning: File: " << filename << " not found." << std::endl;
	}
}

ConfigParser::~ConfigParser()
{
	if (configFile.is_open())
		configFile.close(); 
}

const std::string& ConfigParser::getFilename() const
{
	return filename;
}

bool ConfigParser::isValid() const
{
	return valid;
}

bool ConfigParser::hasKey(const std::string& key)
{
  	bool found = false;
	if (configFile.is_open())
	{
		configFile.clear();
		configFile.seekg(0, std::ios_base::beg);
		std::string beg = key + '=';
		std::string line;
		while (!configFile.eof())
		{
			getline(configFile, line);
			if (beg.compare(line.substr(0, beg.length())) == 0) // line begins with key + "="
			{
				found = true;
				break;
			}
		}
	}
	else
	{
		std::cout << "ConfigParser::hasKey(const std::string& key): Error: No Config-File." << std::endl;
	}
	return found;
}

std::string ConfigParser::getStringKeyValue(const std::string& key)
{
  	std::string keyValue = "";
  	bool found = false;
	if (configFile.is_open())
	{
		configFile.clear();
		configFile.seekg(0, std::ios_base::beg);
		std::string beg = key + '='; 
		std::string line;
		while (!configFile.eof())
		{
			getline(configFile, line);
			// debug output
			// std::cout << line << std::endl;
			if (beg.compare(line.substr(0, beg.length())) == 0) // line begins with key + "="
			{
				keyValue = line.substr(beg.length());
				// debug output
				std::cout << "ConfigParser::getStringKeyValue(const std::string& key): Information: Found " << key << keyValue << std::endl;
				found = true;
				break;
			}
			
		}
		// delete carriage return
		//if(keyValue.at(keyValue.length() - 1) == '\n')
		//	keyValue = keyValue.substr(0, keyValue.length() - 1);
		if (found)
		{
			if (keyValue.at(keyValue.length() - 1) == '\r')
				keyValue = keyValue.substr(0, keyValue.length() - 1);
		}
		else
		{
			std::cout << "ConfigParser::getStringKeyValue(const std::string& key): Information: Key not found " << key << keyValue << std::endl;
			exit(1);
		}


	}
	else
	{
		std::cout << "ConfigParser::getStringKeyValue(const std::string& key): Error: No Config-File." << std::endl;
	}

	return keyValue;
}

int ConfigParser::getIntKeyValue(const std::string& key)
{
  	int keyValue = 0;
	
	std::string temp = getStringKeyValue(key);
	// convert string to int, then return result
	std::istringstream ss(temp);
	ss >> keyValue;
	if (ss.fail())
	{
		// debug output: error
		std::cout << "ConfigParser::getIntKeyValue(const std::string& key): Error: Converting string value to int failed." << keyValue << std::endl;
		keyValue = 0;
	}

	return keyValue;
}

float ConfigParser::getFloatKeyValue(const std::string& key)
{
  	float keyValue = 0.0f;
	
	std::string temp = getStringKeyValue(key);
	// convert string to float, then return result
	std::istringstream ss(temp);
	ss >> keyValue;
	if (ss.fail())
	{
		// debug output: error
		std::cout << "ConfigParser::getFloatKeyValue(const std::string& key): Error: Converting string value to float failed." << keyValue << std::endl;
		keyValue = 0.0f;
	}

	return keyValue;
}

double ConfigParser::getDoubleKeyValue(const std::string& key)
{
  	double keyValue = 0.0;
	
	std::string temp = getStringKeyValue(key);
	// convert string to double, then return result
	std::istringstream ss(temp);
	ss >> keyValue;
	if (ss.fail())
	{
		// debug output: error
		std::cout << "ConfigParser::getDoubleKeyValue(const std::string& key): Error: Converting string value to double failed." << keyValue << std::endl;
		keyValue = 0.0;
	}

	return keyValue;
}

// static interface

bool ConfigParser::isFileValid(const std::string& filename)
{
	ConfigParser parser(filename);
	return parser.isValid();
}

std::string ConfigParser::getStringKeyValue(const std::string& filename, const std::string& key)
{
	ConfigParser parser(filename);
	return parser.getStringKeyValue(key);
}

int ConfigParser::getIntKeyValue(const std::string& filename, const std::string& key)
{
	ConfigParser parser(filename);
	return parser.getIntKeyValue(key);
}

float ConfigParser::getFloatKeyValue(const std::string& filename, const std::string& key)
{
	ConfigParser parser(filename);
	return parser.getFloatKeyValue(key);
}

double ConfigParser::getDoubleKeyValue(const std::string& filename, const std::string& key)
{
	ConfigParser parser(filename);
	return parser.getDoubleKeyValue(key);
}
