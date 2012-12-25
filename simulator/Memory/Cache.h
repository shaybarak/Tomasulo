#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"

class Cache {
public:
	Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: buffer(buffer), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay),
		  previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel) {}
	int getHitCount() { return hits; }
	int getMissCount() { return misses; }
	double getHitRate() { return (double)hits / (hits + misses); }

private:
	// Invalid tag
	static const int INVALID = -1;
	int* buffer;
	int blockSize;
	int cacheSize;
	int accessDelay;
	// Statistics
	int hits;
	int misses;
	// Interfaces to previous & next level
	PreviousMemoryLevel* previousMemoryLevel;
	NextMemoryLevel* nextMemoryLevel;
};
