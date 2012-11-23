#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

class Configuration {
public:
	// Loads from file, returns whether successful
	bool load(std::ifstream& inputFile);
	// Reads value for key, returns whether mapping is present
	bool get(std::string& key, int& value);

private:
	std::map<std::string, int> configs;
};