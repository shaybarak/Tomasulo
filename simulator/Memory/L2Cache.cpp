#include "L2Cache.h"

L2Cache::L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {
	// Also initialize dirty bits
	instructionsDirty.resize(instructionsValid.size());
	dataDirty.resize(dataValid.size());
}

void L2Cache::onTick(int now) {
	// TODO
}
