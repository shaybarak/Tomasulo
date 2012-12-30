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
		pendingWrites::iterator writeAllocate = pendingWrites.find(address);
		if (writeAllocate != pendingWrites::end()) {
			// Was waiting for this word for write-allocate, now send write
			address = *writeAllocate.address;
			data = *writeAllocate.data;
			write(address, data);
			nextMemoryLevel->requestWrite(address, data, now);
			pendingWrites.erase(writeAllocate);
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
			int baseOfBlock = address - (address % blockSize);
			for (int i = 1; i < blockSize / sizeof(int); i++) {
				int fillAddress = baseOfBlock + ((address + i * sizeof(int)) % blockSize);
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
			}
		}
	}

	// Get write request from CPU
	if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
		// Write-allocate so first make sure that block is present in cache
		if ((pendingReadsInternal.find(address) != pendingReadsInternal.end()) ||
			(pendingReadsExternal.find(address) != pendingReadsExternal.end())) {
			// There is a pending read, so delay write until read returns
			hits++;
			Cache::PendingWrite writeAllocate;
			writeAllocate.address = address;
			writeAllocate.data = data;
			// Write critical word first
			pendingWrites.insert(writeAllocate);
		} else if (read(address, &data)) {
			// Satisfied read from cache
			hits++;
			write(address, data);
			nextMemoryLevel->requestWrite(address, data, now + accessDelay);
		} else {
			// Need to read from next level
			misses++;
			// Critical word first
			nextMemoryLevel->requestRead(address, now + accessDelay);
			// Note that the critical word is not a requested read
			pendingReadsInternal.insert(address);
			// Read rest of block
			int baseOfBlock = address - (address % blockSize);
			for (int i = 1; i <= blockSize / sizeof(int); i++) {
				int fillAddress = baseOfBlock + ((address + i * sizeof(int)) % blockSize);
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
			}
			// Write critical word first
			pendingWrites.insert(writeAllocate);
		}
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
