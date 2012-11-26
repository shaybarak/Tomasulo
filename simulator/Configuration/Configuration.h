#pragma once

#include <map>
#include <string>
#include <istream>
#include <regex>
using namespace std;
using namespace std::tr1;

/**
 * Represents a generic configuration in key-value format.
 * Supports loading the configuration from an input stream where each line is formatted as "key = value".
 * Ignores C++ comments (//...).
 */
class Configuration {
public:
	// Loads from stream, returns whether successful
	bool load(std::istream& in);
	// Reads value for key, returns whether mapping is present
	bool get(std::string& key, int* value) const;

private:
	// Line with key and value, possible trailing comment; group 1 matches key, group 2 matches value
	static regex configLine("(\\w*) = (\\d*)(?:\\s*//.*)?");
	// Line with comment only
	static regex commentLine("\\s*//.*");
	std::map<std::string, int> configs;
};