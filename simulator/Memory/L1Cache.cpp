#include "L1Cache.h"
#include "../MIPS32/ISA.h"

void L1Cache::onTick(int now) {
	// TODO
}

bool L1Cache::read(int address, int* value) {
	if (pendingReads.find(address) != pendingReads.end()) {
		// There is a pending read to this address, hence it's invalid
		return false;
	}
	if (address < ISA::CODE_BASE) {
		// Use instructions buffer
		*value = instructions[toBlockNumber(address) + (address % sizeof(int))]
		// Verify valid & tag
		return instructionsValid[toBlockNumber(address)]
			&& (instructionsTag[toBlockNumber(address)] == toTag(address));
	} else {
		*value = data[toBlockNumber(address) + (address % sizeof(int))]
		// Verify valid & tag
		return dataValid[toBlockNumber(address)]
			&& (instructionsTag[toBlockNumber(address)] == toTag(address));
	}
}

void L1Cache::write(int address, int value) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);

	if (address < ISA::CODE_BASE) {
		// Use instructions cache
		instructions[blockNumber] = value;
		instructionsTag[blockNumber] = tag;
		instructionsValid[blockNumber] = true;
	} else {
		// Use data cache
		data[blockNumber] = value;
		dataTag[blockNumber] = tag;
		dataValid[blockNumber] = true;
	}
	// If address was pending a read, no need to wait
	pendingReads.erase(address);
}

int L1Cache::toTag(int address) {
	address / (cacheSize / 2);
}

int L1Cache::toBlockNumber(int address) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is direct-mapped.
	(address / blockSize) % ((cacheSize / 2) / blockSize);
}
