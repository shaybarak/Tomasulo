#pragma once

#include <vector>
#include <ostream>
using namespace std;

class Memory {
public:
	// Constructs with next level memory/cache (may be NULL if this is a terminator)
	Memory(int sizeInBytes, int accessDelay);
	// Read memory word, returns whether successful
	virtual bool read(int address, int* value) = 0;
	// Write memory word, returns whether successful
	virtual bool write(int address, int value) = 0;
	// Dump contents to stream
	bool dump(FILE* out);

private:
	// Memory contents
	vector<int> values;
	// Access delay in clock ticks
	int accessDelay;
};
