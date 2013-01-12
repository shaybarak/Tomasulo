#pragma once

#include <map>
#include <set>
#include <vector>
#include "MasterSlaveInterface.h"

using namespace std;

class Cache {
public:
	Cache(int blockSize, int cacheSize, int accessDelay, int ways = 0);
	const vector<unsigned char>* getBuffer() const { return &buffer; }
	int getHitCount() const { return hits; }
	int getMissCount() const { return misses; }
	double getHitRate() const { return (double)hits / (hits + misses); }

	enum outcome {
		PRESENT,	// Address is present in cache
		CONFLICT,	// Cache has memory block from other address(es) in respective mapping
		INVALID,	// Cache is invalid in respective mapping
	};

	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * data: memory value to write.
	 * May overwrite previous data.
	 */
	virtual void write(int address, int value) = 0;

	/**
	 * Instructs cache to invalidate a block by address.
	 * Used by L2 cache to maintain inclusivity of L1.
	 * On hit invalidates and returns true, on miss returns false.
	 */
	virtual bool invalidate(int address) = 0;

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
