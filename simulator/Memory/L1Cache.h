#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	virtual void onTick(int now);

private:
	/**
	 * Reads a memory word from cache.
	 * If invalid does not send a read request to the next level.
	 *
	 * offset: byte offset
	 * value: output
	 * Returns whether valid. 
	 */
	bool read(int offset, int* value);
	
	/**
	 * Writes a memory word to cache.
	 * Does not send a write request to the next level.
	 *
	 * offset: byte offset
	 * value: input
	 */
	void write(int offset, int value);
};
