#pragma once

#include "Cache.h"

class L1Cache : public Cache {
public:
	L1Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay)
		: Cache(memoryType, blockSize, cacheSize, accessDelay) {}

	// Returns whether address is present in cache
	virtual bool isPresent(int address) = 0;

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

	// Invalidates a block at an address, returns whether was present
	bool invalidate(int address);
};
