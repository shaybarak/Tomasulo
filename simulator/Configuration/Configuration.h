#pragma once

#include <map>
#include <string>
#include <iostream>
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
	bool load(istream& in);
	// Reads value for key, returns whether mapping is present
	bool get(const string& key, int* value) const;

private:
	// Line with key and value, possible trailing comment; group 1 matches key, group 2 matches value
	static const regex configLine("^\\s*(\\w*) = (\\d*)(?:\\s*//.*)?$");
	// Line with comment only
	static const regex commentLine("^\\s*\\s*//.*$");
	map<std::string, int> configs;
};