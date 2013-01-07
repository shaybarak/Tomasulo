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
	return (address / blockSize) % (cacheSize / blockSize / ways);
}

int Cache::toTag(int address) {
	return address / cacheSize * ways;
}

int Cache::toAddress(int tag, int index, int offset) {
	(tag * (cacheSize / ways)) + (index * blockSize) + offset;
}

int Cache::toInstructionsBlock(int index, int way) {
	// Instructions are buffered way 0 first, then way 1 etc'
	return way * instructions.size() / ways + index;
}

int Cache::toInstructionsBlock(int index) {
	return Cache::toInstructionsBlock(index, 0);
}

int Cache::toDataBlock(int index, int way) {
	// Data is buffered such that multiple ways are interleaved per index
	return index * ways + way;
}

int Cache::toDataBlock(int index) {
	return Cache::toDataBlock(index, 0);
}
	
int* Cache::getInstructionPtr(int index, int way, int offset) {
	return &instructions[(toInstructionsBlock(index, way) * blockSize + offset) / sizeof(int)];
}

int* Cache::getInstructionPtr(int index, int offset) {
	return Cache::getInstructionPtr(index, 0, offset);
}

int* Cache::getDataPtr(int index, int way, int offset) {
	return &data[(toDataBlock(index, way) * blockSize + offset) / sizeof(int)];
}

int* Cache::getDataPtr(int index, int offset) {
	return Cache::getDataPtr(index, 0, offset);
}
