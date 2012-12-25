#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"

class Cache {
public:
	Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel);
	const vector<char>* getInstructionsBuffer() const { return &instructionsBuffer; }
	const vector<char>* getDataBuffer() const { return &dataBuffer; }
	int getHitCount() const { return hits; }
	int getMissCount() const { return misses; }
	double getHitRate() const { return (double)hits / (hits + misses); }

private:
	// Invalid tag
	static const int INVALID = -1;
	// Dimensions
	int blockSize;
	int cacheSize;
	int accessDelay;
	// Buffers
	vector<char> instructionsBuffer;
	int* instructions;
	vector<char> dataBuffer;
	int* data;
	// Statistics
	int hits;
	int misses;
	// Interfaces to previous & next level
	PreviousMemoryLevel* previousMemoryLevel;
	NextMemoryLevel* nextMemoryLevel;
};
