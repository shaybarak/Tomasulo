#pragma once

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

/**
 * Represents a generic configuration in key-value format.
 * Supports loading the configuration from a file where each line is formatted as "key=value".
 */
class Configuration {
public:
	// Loads from file, returns whether successful
	bool load(std::ifstream& inputFile);
	// Reads value for key, returns whether mapping is present
	bool get(std::string& key, int* value) const;

private:
	std::map<std::string, int> configs;
};