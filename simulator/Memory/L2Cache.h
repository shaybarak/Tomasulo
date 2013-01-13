#pragma once

#include "Cache.h"

class L2Cache : public Cache {
public:
	L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay);

	// Returns whether address is present in cache
	virtual bool isPresent(int address) = 0;

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data. Precede with a call to isPresent.
	 */
	virtual int read(int address) = 0;
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * value: memory value to write.
	 * way: destination way (when relevant).
	 * dirty: whether this write dirties the cache (when relevant).
	 * May overwrite previous data.
	 */
	virtual void write(int address, int value, int way, bool dirty);

	// Returns LRU way mapped to address
	int getLruWay(int address);

	// Returns whether block mapped to address at given way is valid
	bool isValid(int address, int way);

	// Returns conflicting address (address of data at block for given address and way)
	int getConflictingAddress(int address, int way);

	// Returns whether a block at an address is dirty (assumes address is present)
	bool isDirty(int address);

private:
	vector<bool> dirty;
	vector<int> lruWay;

	bool isPresentInWay(int address, int way);
};

// TODO include offset in getConflictingAddress
// TODO assert present in isDirty