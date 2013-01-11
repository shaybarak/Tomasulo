#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, WAYS, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	// For reading signals from CPU and sending signals to L2 cache
	virtual void onTickUp(int now);
	// For reading signals from L2 cache and sending signals to CPU
	virtual void onTickDown(int now);

	// L2Cache is allowed private access to L1Cache
	friend class L2Cache;

protected:
	typedef Cache::outcome outcome;
	virtual outcome isPresent(int addressIn, int* addressOut);
	virtual void evict(int address);

private:
	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data, users should precede with a call to isPresent.
	 */
	int read(int address);
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * data: memory value to write.
	 * May overwrite previous data, users should precede with a call to isPresent.
	 */
	void write(int address, int value);
	
	static const int WAYS = 1;
};
