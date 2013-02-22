#include "Trace.h"

void Trace::write(string& operation, int timeIssued, int timeExecutionStarted, int timeWriteCdb, int timeCommit) {
	(*out) << operation << "\t" << timeIssued << " " << timeExecutionStarted << " " << timeWriteCdb << " " << timeCommit;
}
