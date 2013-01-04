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

bool Cache::evict(int address) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);
	bool evicted = false;

	if (ISA::isCodeAddress(address)) {
		// Evict from instructions cache if present
		if (instructionsTag[blockNumber] == tag) {
			evicted = instructionsValid[blockNumber];
			instructionsValid[blockNumber] = false;
		}
	} else {
		// Evict from data cache if present
		if (dataTag[blockNumber] == tag) {
			evicted = dataValid[blockNumber];
			dataValid[blockNumber] = false;
		}
	}

	return evicted;
}
