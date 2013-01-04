#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"
#include <map>
#include <set>

using namespace std;

class Cache {
public:
	Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel);
	const vector<unsigned char>* getInstructionsBuffer() const { return &instructionsBuffer; }
	const vector<unsigned char>* getDataBuffer() const { return &dataBuffer; }
	int getHitCount() const { return hits; }
	int getMissCount() const { return misses; }
	double getHitRate() const { return (double)hits / (hits + misses); }

protected:
	/**
	 * Evicts a block from cache by address.
	 * Returns whether address was present in the cache and was evicted.
	 */
	virtual bool evict(int address) = 0;
	// Maps an address to a tag
	virtual int toTag(int address);
	// Maps a memory address to a block number
	virtual int toBlockNumber(int address);
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
	// Memory addresses that have pending reads internally-requested (to fill block)
	set<int> pendingReadsInternal;
	// Memory addresses that have pending reads externally-requested (to serve lower level)
	set<int> pendingReadsExternal;
	// Writes that are pending due to write-allocate
	map<int, int> pendingWrites;

private:
	// Internal buffers
	vector<unsigned char> instructionsBuffer;
	vector<unsigned char> dataBuffer;
};
