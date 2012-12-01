#include "Configuration.h"
#include <utility>
#include <stdlib.h>
using namespace std;

const regex Configuration::configLine("^\\s*(\\w*) = (\\d*)(?:\\s*//.*)?$");
const regex Configuration::commentLine("^\\s*//.*$");
const regex Configuration::emptyLine("^\\s*");

bool Configuration::load(istream& in) {
	string line;
	smatch match;
	while (in) {
		getline(in, line);
		if (regex_search(line, match, configLine)) {
			// Parsed a configuration line
			configs.insert(make_pair(match[1], atoi(match[2].str().c_str())));
		} else if (regex_match(line, commentLine)) {
			// Matched a commented line, ignore
		} else {
			cerr << "Invalid format: " << line << endl;
			return false;
		}
	}
	return true;
}

bool Configuration::get(const string& key, int* value) const {
	map<string, int>::const_iterator it = configs.find(key);
	if (it == configs.end()) {
		cerr << "Missing key " << key << endl;
		return false;
	}
	*value = it->second;
	return true;
}