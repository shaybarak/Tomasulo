#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	virtual void onTick(int now);
};
