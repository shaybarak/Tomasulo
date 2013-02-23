#include "Trace.h"

void Trace::write(const string& operation, int timeIssued, int timeExecutionStarted, int timeWriteCdb, int timeCommit) {
	(*out) << operation << "\t" << timeIssued << " " << timeExecutionStarted << " " << timeWriteCdb << " " << timeCommit << endl;
}
