#pragma once

#include "Cache.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	virtual void onTick(int now);
};
