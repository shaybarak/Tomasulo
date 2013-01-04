#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache);
	virtual void onTick(int now);

protected:
	virtual int toTag(int address);
	virtual int toBlockNumber(int address);

private:
	// Maps an address to a tag
	int toTag(int address);
	// Maps a memory address to a block number and way (way is 0 or 1)
	int toBlockNumber(int address);
	int toWayInstruction(int blockNumber, int way);
	int toWayData(int blockNumber, int way);
	bool read(int address, int* value);
	void write(int address, int value);

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;

	// Associated L1 cache
	L1Cache* l1Cache;
};
