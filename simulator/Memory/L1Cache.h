#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	virtual void onTick(int now);

	//L2Cache willons use protected functions of L1Cahce and vice versa
	friend class L2Cache;

protected:
	virtual bool evict(int address);
	virtual int toTag(int address);
	virtual int toBlockNumber(int address);
	virtual int toAddress(int tag, int blockNumber, int blockOffset);

private:
	/**
	 * Reads a memory word from cache.
	 * If invalid does not send a read request to the next level.
	 *
	 * address: byte offset
	 * value: output - data in associated block even if not valid or tag mismatch
	 * Returns whether valid. 
	 */
	bool read(int address, int* value);

	/**
	 * Writes a memory word to cache.
	 * Does not send a write request to the next level.
	 *
	 * address: byte offset
	 * value: input
	 */
	void write(int address, int value);
};
