#include "L1Cache.h"
#include "../MIPS32/ISA.h"

void L1Cache::onTick(int now) {
	int address, data;

	// Get read response from L2 cache
	if (nextMemoryLevel->getReadResponse(&address, &data, now)) {
		write(address, value);
		pendingReadsInternal.erase(address);
		if (pendingReadsExternal.erase(address) > 0) {
			// Serve incoming data to lower level
			previousMemoryLevel->respondRead(address, data, now);
		}
	}

	// Get read request from CPU
	if (previousMemoryLevel->getReadRequest(&address, now)) {
		if ((pendingReadsInternal.find(address) != pendingReadsInternal.end()) ||
			(pendingReadsExternal.find(address) != pendingReadsExternal.end())) {
			// There is a pending read, so delay but mark this as a hit
			hits++;
		} else if (read(address, &data)) {
			// Satisfied read from cache
			hits++;
			previousMemoryLevel->respondRead(address, data, now + accessDelay);
		} else {
			// Need to read from next level
			misses++;
			// Critical word first
			nextMemoryLevel->requestRead(address, now + accessDelay);
			pendingReadsExternal.insert(address);
			// Read rest of block
			// TODO loop around all values at sizeof(int) increments cyclically not including original address
		}
	}

	// Get write request from CPU
	if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
		// TODO
	}
}

bool L1Cache::read(int address, int* value) {
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
}

int L1Cache::toTag(int address) {
	address / (cacheSize / 2);
}

int L1Cache::toBlockNumber(int address) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is direct-mapped.
	(address / blockSize) % ((cacheSize / 2) / blockSize);
}
