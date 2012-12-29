#pragma once

#include "Cache.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel);
	virtual void onTick(int now);

private:
	// Maps an address to a tag
	int toTag(int address);
	// Maps a memory address to a block number and way (way is 0 or 1)
	int toBlockNumber(int address);
	int toWayInstruction(int blockNumber, int way);
	int toWayData(int blockNumber, int way);

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;
};
