#pragma once

#include "..\MIPS32\ISA.h"
#include <ostream>
using namespace std;

/**
 * Dumps registers to an output stream (e.g. ofstream to write to a file).
 * Sample output:
 * $0 0
 * $3 1
 * ...
 * $31 14
 * (output is in decimal digits)
 */
class RegisterDump {
public:
	// Returns whether successful
	static bool dump(ISA::Register* registers, int count, ostream& out);
};