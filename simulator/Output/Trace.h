#pragma once

#include <ostream>
#include <string>
using namespace std;

class Trace {
public:
	// Initializes with an output stream to write trace information to.
	Trace(ostream& out) : out(&out) {}
	// Writes a trace entry
	void write(string& operation, int timeIssued, int timeExecutionStarted, int timeWriteCdb, int timeCommit);

private:
	ostream* out;
};
