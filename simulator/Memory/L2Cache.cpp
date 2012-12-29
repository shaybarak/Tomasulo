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

int L2Cache::toTag(int address) {
	// Since there are two ways per block in the cache
	address / (cacheSize / 4);
}

int L2Cache::toInstructionBlockNumber(int address, int way) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is direct-mapped.
	// Ways are sequential in the data cache (first all of way 0, then all of way 1).
	(address / blockSize) % ((cacheSize / 4) / blockSize) + ((cacheSize / 4) / blockSize * way);
}

int L2Cache::toDataBlockNumber(int address, int way) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is direct-mapped.
	// Ways are interleaved in the data cache (way 0, way 1, way 0, way 1...)
	((address / blockSize) % ((cacheSize / 4) / blockSize)) + way;
}
