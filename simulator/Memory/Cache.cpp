#include "Cache.h"

Cache::Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int ways)
		: memoryType(memoryType), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay), ways(ways),
		  hits(0), misses(0) {
	buffer.resize(cacheSize);
	// Tag and valid bit are per block
	tags.resize(buffer.size() / sizeof(int));
	valid.resize(buffer.size() / sizeof(int));
	// Use vector<unsigned char> as scoped int buffer
	words = (int*)&buffer[0];
}

/**
 * Address:
 * +----------------+-------+--------+
 * | Tag            | Index | Offset |
 * +----------------+-------+--------+
 */

int Cache::toOffset(int address) {
	return address % sizeof(int);
}

int Cache::toIndex(int address) {
	// Divide address by block size to trim offset part.
	// Compute modulus of how many possible indices there are.
	// For each index we consume a block times how many ways there are per entry.
	return (address / sizeof(int)) % (cacheSize / sizeof(int) * ways);
}

int Cache::toTag(int address) {
	return address / (cacheSize / ways);
}

int Cache::toAddress(int tag, int index, int offset) {
	return (tag * (cacheSize / ways)) + (index * sizeof(int)) + offset;
}

int Cache::toBlock(int index, int way) {
	if (memoryType == ISA::INST) {
		// The buffer is used such that all of way 0 is saved before way 1
		return index + (way * cacheSize / sizeof(int) / ways);
	} else {
		// The ways are interleaved, for each index first way 0 is saved and then way 1
		return (index * ways) + way;
	}
}

int* Cache::getWordPtr(int index, int offset, int way) {
	return &words[(toBlock(index, way) * sizeof(int) + offset) / sizeof(int)];
}
