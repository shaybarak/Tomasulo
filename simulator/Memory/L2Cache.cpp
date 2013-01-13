#include "L2Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

L2Cache::L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int l1BlockSize,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave)
		: Cache(memoryType, blockSize, cacheSize, accessDelay, 2), l1BlockSize(l1BlockSize),
		  pL1Master(pL1Master), pRamSlave(pRamSlave), state(READY), delay(-1) {
	// Also initialize dirty bits
	dirty.resize(valid.size());
	// Also initialize LRU bits (2 ways per block)
	way0IsLru.resize(valid.size() / 2);
}

bool L2Cache::isPresentInWay(int address, int way) {
	int tag = toTag(address);
	int index = toIndex(address);
	int block = toBlock(index, way);
	return (valid[block] && (tags[block] == tag));
}

bool L2Cache::isHit(int address) {
	return (isPresentInWay(address, 0) || isPresentInWay(address, 1));
}

int L2Cache::findVacancy(int address) {
	int tag = toTag(address);
	int index = toIndex(address);
	if (!valid[toBlock(index, 0)]) {
		return 0;
	} else if (!valid[toBlock(index, 1)]) {
		return 1;
	} else {
		return -1;
	}
}

int L2Cache::read(int address) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	if (isPresentInWay(address, 0)) {
		way0IsLru[index] = false;
		return *getWordPtr(index, offset, 0);
	} else {
		way0IsLru[index] = true;
		return *getWordPtr(index, offset, 1);
	}
}

void L2Cache::write(int address, int value) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	if (isPresentInWay(address, 0)) {
		way0IsLru[index] = false;
		// Write is from CPU
		dirty[toBlock(index, 0)] = true;
		*getWordPtr(index, offset, 0) = value;
	} else {
		assert(isPresentInWay(address, 1));
		// Assumes present in second way
		way0IsLru[index] = true;
		dirty[toBlock(index, 1)] = true;
		*getWordPtr(index, offset, 1) = value;
	}
}

void L2Cache::write(int address, int value, int way) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	int block = toBlock(index, way);
	*getWordPtr(index, offset, way) = value;
	tags[block] = tag;
	// Write is from RAM
	dirty[block] = false;
	valid[block] = true;
	if (way == 0) {
		way0IsLru[index] = false;
	} else {
		way0IsLru[index] = true;
	}
}

int L2Cache::nextAddress(int address, int blockSize) {
	// Implements the cyclic address iteration
	return ((address / blockSize) * blockSize) + ((address + sizeof(int)) % blockSize);
}
