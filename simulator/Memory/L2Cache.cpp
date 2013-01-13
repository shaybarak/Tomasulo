#include "L2Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

L2Cache::L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay)
		: Cache(memoryType, blockSize, cacheSize, accessDelay, 2) {
	// Also initialize dirty bits
	dirty.resize(valid.size());
	// Also initialize LRU way (contains way number of LRU way but represents an actual bit in the cache's buffer)
	lruWay.resize(valid.size() / 2);  // Note that LRU is initialized to 0
}

bool L2Cache::isPresentInWay(int address, int way) {
	int tag = toTag(address);
	int index = toIndex(address);
	int block = toBlock(index, way);
	return (valid[block] && (tags[block] == tag));
}

int L2Cache::getPresentWay(int address) {
	for (int way = 0; way < ways; way++) {
		if (isPresentInWay(address, way)) {
			return way;
		}
	}
	return -1;
}

int L2Cache::getInvalidWay(int address) {
	for (int way = 0; way < ways; way++) {
		if (!isValid(address, way)) {
			return way;
		}
	}
	return -1;
}

bool L2Cache::isPresent(int address) {
	return (isPresentInWay(address, 0) || isPresentInWay(address, 1));
}

int L2Cache::read(int address) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	if (isPresentInWay(address, 0)) {
		lruWay[index] = 1;
		return *getWordPtr(index, offset, 0);
	} else {
		//assert(isPresentInWay(address, 1));
		lruWay[index] = 0;
		return *getWordPtr(index, offset, 1);
	}
}

void L2Cache::write(int address, int value, int way, bool isDirty) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	int block = toBlock(index, way);
	lruWay[index] = 1 - way;  // The LRU way is not the other one
	*getWordPtr(index, offset, way) = value;
	valid[block] = true;
	tags[block] = tag;
	dirty[block] = isDirty;
}

int L2Cache::getLruWay(int address) {
	return lruWay[toIndex(address)];
}

bool L2Cache::isValid(int address, int way) {
	return valid[toBlock(toIndex(address), way)];
}

int L2Cache::getConflictingAddress(int address, int way) {
	int offset = toOffset(address);
	int index = toIndex(address);
	int block = toBlock(index, way);
	int tag = tags[block];
	return toAddress(tag, index, offset);
}

bool L2Cache::isDirty(int address) {
	int way;
	if (isPresentInWay(address, 0)) {
		way = 0;
	} else {
		// Address must be present in one of the ways
		assert(isPresentInWay(address, 1));
		way = 1;
	}
	return dirty[toBlock(toIndex(address), way)];
}
