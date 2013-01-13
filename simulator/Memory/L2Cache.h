#pragma once

#include "Cache.h"

class L2Cache : public Cache {
public:
	L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay);

	// Returns whether address is present in cache
	virtual bool isPresent(int address);

	// Returns whether address is present in cache at given way
	bool isPresentInWay(int address, int way);

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data. Precede with a call to isPresent.
	 */
	int read(int address);
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * value: memory value to write.
	 * isDirty: whether this write dirties the cache.
	 * May overwrite previous data.
	 */
	void write(int address, int value, bool isDirty);

	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * value: memory value to write.
	 * isDirty: whether this write dirties the cache (when relevant).
	 * way: destination way.
	 * May overwrite previous data.
	 */
	void write(int address, int value, int way, bool isDirty);

	// Returns LRU way mapped to address
	int getLruWay(int address);

	// Returns whether block mapped to address at given way is valid
	bool isValid(int address, int way);

	// Returns conflicting address (address of data at block for given address and way)
	int getConflictingAddress(int address, int way);

	// Returns whether a block at an address is dirty (assumes address is present)
	bool isDirty(int address);

	//returns way where address is present, -1 if not present at all
	int getPresentWay(int address);

private:
	vector<bool> dirty;
	vector<int> lruWay;

};
