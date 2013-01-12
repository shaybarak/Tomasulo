#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"
#include <map>
#include <set>

using namespace std;

class Cache {
public:
	Cache(int blockSize, int cacheSize, int accessDelay, int ways = 0);
	const vector<unsigned char>* getInstructionsBuffer() const { return &instructionsBuffer; }
	const vector<unsigned char>* getDataBuffer() const { return &dataBuffer; }
	int getHitCount() const { return hits; }
	int getMissCount() const { return misses; }
	double getHitRate() const { return (double)hits / (hits + misses); }

	enum outcome {
		PRESENT,	// Address is present in cache
		CONFLICT,	// Cache has memory block from other address(es) in respective mapping
		INVALID,	// Cache is invalid in respective mapping
	};

protected:
	// Returns block offset of address
	int toOffset(int address);
	// Returns index of address
	int toIndex(int address);
	// Returns tag of address
	int toTag(int address);
	// Builds a memory address out of the tag, index and offset
	int toAddress(int tag, int index, int offset);
	// Maps to cache block space
	int toBlock(int index, int way = 0);
	// Returns pointer to word from cache
	int* getWordPtr(int index, int offset, int way = 0);
	
	// Dimensions
	int blockSize;
	int cacheSize;
	int accessDelay;
	int ways;
	
	// Buffers
	int* words;
	vector<int> tags;
	vector<bool> valid;
	
	// Statistics
	int hits;
	int misses;

private:
	// Internal buffer
	vector<unsigned char> buffer;
};
