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
	// Maps a memory address to a block number (way is 0 or 1)
	int toInstructionBlockNumber(int address, int way);
	int toDataBlockNumber(int address, int way);

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
};
