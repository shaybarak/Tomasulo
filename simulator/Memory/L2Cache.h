#pragma once

#include "Cache.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel, int sizeOfBlock)
		: Cache(previousMemoryLevel, nextMemoryLevel, sizeOfBlock) {}
	virtual void onTick(int now);
};
