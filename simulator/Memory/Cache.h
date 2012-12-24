#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"

class Cache {
public:
	Cache(PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel, int sizeOfBlock)
		: previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel), sizeOfBlock(sizeOfBlock) {}
	int getHitCount() { return hits; }
	int getMissCount() { return misses; }
	double getHitRate() { return (double)hits / (hits + misses); }

private:
	// Invalid tag
	static const int INVALID = -1;
	int sizeOfBlock;
	// Statistics
	int hits;
	int misses;
	// Interfaces to previous & next level
	PreviousMemoryLevel* PreviousMemoryLevel;
	NextMemoryLevel* nextMemoryLevel;
};
