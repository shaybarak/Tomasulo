#include "Cache.h"

Cache::Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: data(buffer), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay),
		  previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel) {
	instructionsBuffer.resize(cacheSize / 2);
	// Tag and valid bit are per block
	instructionsTag.resize(instructionsBuffer.size() / blockSize);
	instructionsValid.resize(instructionsTag.size());
	instructions = (int*)&instructionsBuffer[0];
	dataBuffer.resize(cacheSize / 2);
	// Tag and valid bit are per block
	dataTag.resize(dataBuffer.size() / blockSize);
	dataValid.resize(dataTag.size());
	data = (int*)&dataBuffer[0];
}
