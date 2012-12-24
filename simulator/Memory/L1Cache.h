#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel, int sizeOfBlock)
		: Cache(previousMemoryLevel, nextMemoryLevel, sizeOfBlock) {}
	virtual void onTick(int now);
};
