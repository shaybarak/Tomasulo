#pragma once

#include <vector>
#include <stdio.h>
#include <fstream>
using namespace std;

/**
 * Works with snapshots of a byte array.
 * Reads from or writes to a stream (e.g. dump to output file).
 * Format is uppercase hexadecimal bytes with every 8 bytes separated by a new line.
 * Sample:
 * FA BB 38 55 4A 3A BE 00
 * 44 DE AD FE ED 65 CA 22
 */
class HexDump {
public:
	// Reads formatted bytes into buffer, returns whether successful
	static bool load(vector<unsigned char>& buffer, fstream& in);
	// Writes bytes according to format, returns whether successful
	static bool store(const vector<unsigned char>& buffer, fstream& out);
};
