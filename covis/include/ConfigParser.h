// Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

// This file provides an class for parsing a simple config file with one entry
// entry of the form KEY=VALUE per line. A comment lines are allowed and 
// starts with '#'.

#ifndef ConfigParser_h
#define ConfigParser_h

#include <string>
#include <fstream>

class ConfigParser
{
public:
	ConfigParser(const std::string& filename);
	~ConfigParser();
	// interface
	bool isValid() const;
	const std::string& getFilename() const;
	bool hasKey(const std::string& key);
	std::string getStringKeyValue(const std::string& key);
	int getIntKeyValue(const std::string& key);
	float getFloatKeyValue(const std::string& key);
	double getDoubleKeyValue(const std::string& key);
	
	// static interface
	static bool isFileValid(const std::string& filename);
	static std::string getStringKeyValue(const std::string& filename, const std::string& key);
	static int getIntKeyValue(const std::string& filename, const std::string& key);
	static float getFloatKeyValue(const std::string& filename, const std::string& key);
	static double getDoubleKeyValue(const std::string& filename, const std::string& key);
private:
	bool valid; // true if file is valid
	std::ifstream configFile; // represents the config file
	std::string filename;
};

#endif // ConfigParser_h
