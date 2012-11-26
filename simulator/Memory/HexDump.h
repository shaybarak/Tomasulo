#pragma once

#include <ostream>

/**
 * Dumps a memory snapshot to an output stream (e.g. ofstream to write to a file).
 * Output is formatted as uppercase hexadecimal bytes with each byte separated by a new line.
 * Sample output:
 * FA BB 38 55 4A 3A BE 00
 * 44 DE AD FE ED 65 CA 22
 */
class HexDump {
public:
	// Returns whether successful
	static bool dumpToFile(char* buffer, int length, ostream& out);
};