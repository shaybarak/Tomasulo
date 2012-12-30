#include "L2Cache.h"

L2Cache::L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel) {
	// Also initialize dirty bits
	instructionsDirty.resize(instructionsValid.size());
	dataDirty.resize(dataValid.size());
	// Also initialize LRU bits (2 ways per block)
	instructionsWay0IsLru.resize(instructionsValid.size() / 2);
	dataWay0IsLru.resize(dataValid.size() / 2);
}

void L2Cache::onTick(int now) {
	// TODO
}

bool L2Cache::read(int address, int* value) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);

	if (address < ISA::CODE_BASE) {
		// Use instructions buffer
		int way0 = toWayInstruction(blockNumber, 0);
		int way1 = toWayInstruction(blockNumber, 1);
		if (instructionsValid[way0] && (instructionsTag[way0] == toTag(address))) {
			// Present in way 0
			*value = instructions[way0 + (address % sizeof(int))];
			instructionsWay0IsLru[blockNumber] = false;
			return true;
		} else if (instructionsValid[way1] && (instructionsTag[way1] == toTag(address))) {
			// Present in way 1
			*value = instructions[way0 + (address % sizeof(int))];
			instructionsWay0IsLru[blockNumber] = true;
			return true;
		} else {
			// Not present
			return false;
		}
	} else {
		// Use data buffer
		int way0 = toWayData(blockNumber, 0);
		int way1 = toWayData(blockNumber, 1);
		if (dataValid[way0] && (dataTag[way0] == tag)) {
			// Present in way 0
			*value = data[way0 + (address % sizeof(int))];
			dataWay0IsLru[blockNumber] = false;
			return true;
		} else if (dataValid[way1] && (dataTag[way1] == tag)) {
			// Present in way 1
			*value = data[way0 + (address % sizeof(int))];
			dataWay0IsLru[blockNumber] = true;
			return true;
		} else {
			// Not present
			return false;
		}
	}
}

void L2Cache::write(int address, int value) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);

	if (address < ISA::CODE_BASE) {
		// Use instructions cache
		int way;
		if (instructionsWay0IsLru[blockNumber]) {
			// Way 0 is the LRU
			way = toWayInstruction(blockNumber, 0);
		} else {
			// Way 1 is the LRU
			way = toWayInstruction(blockNumber, 1);
		}
		instructions[way] = value;
		instructionsTag[way] = tag;
		instructionsValid[way] = true;
	} else {
		// Use data cache
		int way;
		if (dataWay0IsLru[blockNumber]) {
			// Way 0 is the LRU
			way = toWayData(blockNumber, 0);
		} else {
			// Way 1 is the LRU
			way = toWayData(blockNumber, 1);
		}
		data[way] = value;
		dataTag[way] = tag;
		dataValid[way] = true;
	}
}

int L2Cache::toTag(int address) {
	// Since there are two ways per block in the cache
	address / (cacheSize / 4);
}

int L2Cache::toBlockNumber(int address, int way) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is 2-way set associative.
	(address / blockSize) % ((cacheSize / 4) / blockSize);// + ((cacheSize / 4) / blockSize * way);
}

int L2Cache::toWayInstruction(int blockNumber, int way) {
	// Ways are sequential in the instruction cache (first all of way 0, then all of way 1).
	blockNumber + ((cacheSize / 4) / blockSize * way);
}

int L2Cache::toWayData(int blockNumber, int way) {
	// Ways are interleaved in the data cache (way 0, way 1, way 0, way 1...)
	blockNumber + way;
}
