#pragma once

#include <vector>
#include "../MIPS32/ISA.h"
using namespace std;

class MainMemory {
public:
	MainMemory(ISA::MemoryType memoryType, int accessDelay);

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data. Precede with a call to isPresent.
	 */
	int read(int address);
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * data: memory value to write.
	 * May overwrite previous data.
	 */
	void write(int address, int value);
	
	// Design flaw: exposes non-const pointer to data member
	// (required for loading memory initialization)
	vector<unsigned char>* getBuffer() { return &buffer; }

	int getAccessDelay() { return accessDelay; }

private:
	// Defines whether type is instructions/data
	ISA::MemoryType memoryType;

	vector<unsigned char> buffer;
	int* words;
	int accessDelay;
};
