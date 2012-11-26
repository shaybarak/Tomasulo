#pragma once

#include <ostream>

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
private:
	// Count of registers in ISA
	static const int REGISTER_COUNT = 32;
};