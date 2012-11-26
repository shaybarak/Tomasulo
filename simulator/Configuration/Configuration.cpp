#include "Configuration.h"

static const string equals_token("=");

bool Configuration::load(istream& in) {
	// TODO handle comments
	string line, key;
	int value;
	smatch match;
	while (in.good() && !in.eof()) {
		getline(in, line);
		if (regex_search(line, match, configLine)) {
			// Parsed a configuration line
			configs[match[1]] = match[2];
		} else if (regex_match(line, commentLine)) {
			// Matched a commented line, ignore
		} else {
			cerr << "Invalid format: " << line << endl;
			return false;
		}
	}
	return true;
}

bool Configuration::get(string& key, int* value) const {
	map<string, int>::const_iterator it = configs.find(key);
	if (it == configs.end()) {
		cerr << "Missing key " << key << endl;
		return false;
	}
	*value = it->second;
	return true;
}