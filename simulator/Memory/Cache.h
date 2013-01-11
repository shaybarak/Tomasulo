#pragma once

#include "PreviousMemoryLevel.h"
#include "NextMemoryLevel.h"
#include <map>
#include <set>

using namespace std;

class Cache {
public:
	Cache(int* buffer, int blockSize, int cacheSize, int ways, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel);
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
	/**
	 * Returns whether:
	 * - A memory address is present in the cache
	 * - It's conflicting with another block
	 * - The mapping is invalid.
	 *
	 * addressIn: address to check
	 * addressOut: conflicting address (if returned conflicting)
	 */
	virtual outcome isPresent(int addressIn, int* addressOut) = 0;
	
	/**
	 * Evicts a block from cache by address.
	 * If address was not present in the cache, does nothing.
	 */
	virtual void evict(int address) = 0;
	
	// Returns block offset of address
	int toOffset(int address);
	// Returns index of address
	int toIndex(int address);
	// Returns tag of address
	int toTag(int address);
	// Builds a memory address out of the tag, index and offset
	int toAddress(int tag, int index, int offset);
	// Maps to instructions cache block space
	int toInstructionsBlock(int index, int way = 0);
	// Maps to data cache block space
	int toDataBlock(int index, int way = 0);
	
	// Returns pointer to instruction from cache
	int* getInstructionPtr(int index, int offset, int way = 0);
	
	 //Returns pointer to data from cache
	int* getDataPtr(int index, int offset, int way = 0);
	
	// Dimensions in bytes
	int blockSize;
	int cacheSize;
	int ways;
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
	
	// Memory addresses that have pending reads internally-requested (to fill block).
	// If an incoming request matches any of these addresses it is considered a hit,
	// despite the fact that the value at the address is not currently present in the cache.
	set<int> pendingReadsInternal;
	// Memory addresses that have pending reads externally-requested (to serve lower level).
	// When these pending read operations return, the result will be sent to the lower level.
	set<int> pendingReadsExternal;
	// Writes that are pending due to write-allocate or write-back
	map<int, int> pendingWrites;

private:
	// Internal buffers
	vector<unsigned char> instructionsBuffer;
	vector<unsigned char> dataBuffer;
};
