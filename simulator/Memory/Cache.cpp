#include "Cache.h"

Cache::Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
	PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
	: buffer(buffer), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay),
	  previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel) {
	instructionsBuffer(cacheSize / 2);
	// Valid bits are per block
	instructionsValid(instructionsBuffer.size() / blockSize);
	instructions = &instructionsBuffer[0];
	dataBuffer(cacheSize / 2);
	// Valid bits are per block
	dataValid(dataBuffer.size() / blockSize);
	data = &dataBuffer[0];
}
