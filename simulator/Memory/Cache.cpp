#include "Cache.h"

Cache::Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
	PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
	: buffer(buffer), blockSize(blockSize), cacheSize(cacheSize), accessDelay(accessDelay),
	  previousMemoryLevel(previousMemoryLevel), nextMemoryLevel(nextMemoryLevel) {
	instructionsBuffer(cacheSize / 2);
	instructions = &instructionsBuffer[0];
	dataBuffer(cacheSize / 2);
	data = &dataBuffer[0];
}
