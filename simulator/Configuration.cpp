#include "Configuration.h"

static const string equals_token("=");

bool Configuration::load(ifstream& inputFile) {
	string line, key, equals;
	int value;
	if (!inputFile.is_open()) {
		cerr << "Unable to open file";
		return false;
	}
	while (inputFile.good()) {
		getline(inputFile, line);
		istringstream lineStream(line);
		lineStream >> key >> equals >> value;
		if ((lineStream.rdstate() == ios::failbit) || (equals != equals_token)) {
			cerr << "Invalid format: " << line << endl;
			return false;
		}
	}
	return true;
}

bool Configuration::get(string& key, int& value) const {
	map<string, int>::const_iterator it = configs.find(key);
	if (it == configs.end()) {
		cerr << "Missing key " << key << endl;
		return false;
	}
	value = it->second;
	return true;
}