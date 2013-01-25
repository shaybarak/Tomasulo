#include "Cache.h"

Cache::Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int ways)
		: memoryType(memoryType), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay), ways(ways),
		  hits(0), misses(0) {
	buffer.resize(cacheSize);
	// Tag and valid bit are per block
	tags.resize(buffer.size() / blockSize);
	valid.resize(buffer.size() / blockSize);
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
	return address % blockSize;
}

int Cache::toIndex(int address) {
	// Divide address by block size to trim offset part.
	// Compute modulus of how many possible indices there are.
	// For each index we consume a block times how many ways there are per entry.
	return (address / blockSize) % (cacheSize / (blockSize * ways));
}

int Cache::toTag(int address) {
	return address / (cacheSize / ways);
}

int Cache::toAddress(int tag, int index, int offset) {
	return (tag * (cacheSize / ways)) + (index * blockSize) + offset;
}

int Cache::toBlock(int index, int way) {
	// The exercise instructions state that instruction and data caches are ordered differently
	// (items #10 and #11 in the part 2 instructions), but this is not the case
	// as far as the examples that we were given are concerned!
	//if (memoryType == ISA::INST) {
		// The buffer is used such that all of way 0 is saved before way 1
		return index + (way * cacheSize / (blockSize * ways));
	//} else {
	//	// The ways are interleaved, for each index first way 0 is saved and then way 1
	//	return (index * ways) + way;
	//}
}

int* Cache::getWordPtr(int index, int offset, int way) {
	return &words[(toBlock(index, way) * blockSize + offset) / sizeof(int)];
}
