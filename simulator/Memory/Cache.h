#pragma once

#include <map>
#include <set>
#include <vector>
#include "../MIPS32/ISA.h"

using namespace std;

class Cache {
public:
	Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int ways = 1);
	const vector<unsigned char>* getBuffer() const { return &buffer; }
	
	void registerHit() { hits++; }
	void registerMiss() { misses++; }
	int getHitCount() const { return hits; }
	int getMissCount() const { return misses; }
	int getAccessDelay() { return accessDelay; }
	int getBlockSize() { return blockSize; }
	int getWayCount() { return ways; }

	// Returns whether address is present in cache
	virtual bool isPresent(int address) = 0;

protected:
	// Defines whether type is instructions/data
	ISA::MemoryType memoryType;

	// Returns block offset of address
	int toOffset(int address);
	// Returns index of address
	int toIndex(int address);
	// Returns tag of address
	int toTag(int address);
	// Builds a memory address out of the tag, index and offset
	int toAddress(int tag, int index, int offset);
	// Maps into cache buffer block space
	int toBlock(int index, int way);
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
