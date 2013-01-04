#pragma once

#include "Cache.h"
#include "L1Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache);
	virtual void onTick(int now);

protected:
	virtual bool evict(int address);
	virtual int toTag(int address);
	virtual int toBlockNumber(int address);
	virtual int toAddress(int tag, int blockNumber, int blockOffset);

private:
	int toWayInstruction(int blockNumber, int way);
	int toWayData(int blockNumber, int way);
	/**
	 * Reads a memory word from cache.
	 * If invalid does not send a read request to the next level.
	 *
	 * address: byte offset
	 * value: output - data read if valid or data present in associated LRU way
	 *        (to evict when read from next level returns)
	 * addressToEvict: address of data present in associated LRU way if needs evicting
	 *                 or negative otherwise
	 * Returns whether valid. 
	 */
	bool read(int address, int* value, int* addressToEvict);
	/**
	 * Writes a memory word to cache.
	 * Does not send a write request to the next level.
	 *
	 * address: byte offset
	 * value: input
	 */
	void write(int address, int value);

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;

	// Associated L1 cache
	L1Cache* l1Cache;
};
