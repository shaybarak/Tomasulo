#include "Cache.h"

Cache::Cache(int* buffer, int blockSize, int cacheSize, int ways, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: data(buffer), blockSize(blockSize), cacheSize(cacheSize), ways(ways), accessDelay(accessDelay),
		  previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel),
		  hits(0), misses(0) {
	// Split cache buffer 50%-50% between instructions and data
	instructionsBuffer.resize(cacheSize / 2);
	dataBuffer.resize(cacheSize / 2);
	// Tag and valid bit are per block
	instructionsTag.resize(instructionsBuffer.size() / blockSize);
	dataTag.resize(instructionsTag.size());
	instructionsValid.resize(instructionsTag.size());
	dataValid.resize(dataTag.size());
	// Use vector<unsigned char> as scoped int buffer
	instructions = (int*)&instructionsBuffer[0];
	data = (int*)&dataBuffer[0];
}

int Cache::toOffset(int address) {
	return address % blockSize;
}
	
int Cache::toIndex(int address) {
	return (address / blockSize) % (cacheSize / blockSize / ways);
}

int Cache::toTag(int address) {
	return address / cacheSize * ways;
}

int Cache::toAddress(int tag, int index, int offset) {
	(tag * (cacheSize / ways)) + (index * blockSize) + offset;
}
	
int* Cache::getInstructionPtr(int tag, int index, int way, int offset) {
	// Instructions are buffered way 0 first, then way 1
	return &instructions[(way * instructions.size() / 2 + index * blockSize + offset) / sizeof(int)];
}

int* Cache::getDataPtr(int offset, int index, int way, int tag) {
	// Data is buffered such that way 0 and way 1 are interleaved per index
	return &data[((index * 2 + way) * blockSize + offset) / sizeof(int)];
}
