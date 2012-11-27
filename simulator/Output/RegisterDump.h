#pragma once

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
class HexDump {
public:
	// Registers is expected to be int[32]
	// Returns whether successful
	static bool dump(int* registers, ostream& out);
};