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

protected:
	// Dimensions in bytes
	int blockSize;
	int cacheSize;
	int accessDelay;
	// Buffers
	int* instructions;
	vector<int> instructionsTag;
	vector<bool> instructionsValid;
	int* data;
	vector<int> dataTag;
	vector<bool> dataValid;
	// Statistics
	int hits;
	int misses;
	// Interfaces to previous & next level
	PreviousMemoryLevel* previousMemoryLevel;
	NextMemoryLevel* nextMemoryLevel;
	// Memory addresses that have pending reads
	set<int> pendingReads;

private:
	// Internal buffers
	vector<char> instructionsBuffer;
	vector<char> dataBuffer;
};
