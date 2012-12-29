#pragma once

#include "Cache.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel);
	virtual void onTick(int now);

private:
	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
};
