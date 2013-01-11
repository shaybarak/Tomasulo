#include "L2Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

L2Cache::L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache)
		: Cache(buffer, blockSize, cacheSize, WAYS, accessDelay, previousMemoryLevel, nextMemoryLevel),
		  l1Cache(l1Cache) {
	// Also initialize dirty bits
	instructionsDirty.resize(instructionsValid.size());
	dataDirty.resize(dataValid.size());
	// Also initialize LRU bits (2 ways per block)
	instructionsWay0IsLru.resize(instructionsValid.size() / 2);
	dataWay0IsLru.resize(dataValid.size() / 2);
}

void L2Cache::onTick(int now) {
	int address, data, addressToEvict;

	// Get read response from main memory
	if (nextMemoryLevel->getReadResponse(&address, &data, now)) {
		write(address, data);
		pendingReadsInternal.erase(address);
		if (pendingReadsExternal.erase(address) > 0) {
			// Serve incoming data to lower level as well
			previousMemoryLevel->respondRead(address, data, now);
		}
		
		map<int, int>::iterator writeBack = pendingWrites.find(address);
		if (writeBack != pendingWrites.end()) {
			// Was waiting for this word for write-allocate, now send write
			address = writeBack->first;
			data = writeBack->second;
			write(address, data);
			nextMemoryLevel->requestWrite(address, data, now);
			pendingWrites.erase(writeBack);
		}
	}

	// Get read request from CPU
	if (previousMemoryLevel->getReadRequest(&address, now)) {
		if ((pendingReadsInternal.find(address) != pendingReadsInternal.end()) ||
			(pendingReadsExternal.find(address) != pendingReadsExternal.end())) {
			// There is a pending read, so delay but mark this as a hit
			hits++;
		} else if (read(address, &data, &addressToEvict)) {
			// Satisfied read from cache
			hits++;
			previousMemoryLevel->respondRead(address, data, now + accessDelay);
		} else {
			// Need to read from next level
			misses++;
			// Critical word first
			nextMemoryLevel->requestRead(address, now + accessDelay);
			pendingReadsExternal.insert(address);
			if (addressToEvict >= 0) {
				// Remember to write-back when read returns
				pendingWrites[addressToEvict] = data;
			}
			// Read rest of block
			int baseOfBlock = address - (address % blockSize);
			for (int i = 1; i < blockSize / (int)sizeof(int); i++) {
				int fillAddress = baseOfBlock + ((address + i * sizeof(int)) % blockSize);
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
				read(fillAddress, &data, &addressToEvict);
				if (addressToEvict >= 0) {
					pendingWrites[addressToEvict] = data;
				}
			}
		}
	}

	// Get write request from L1 cache
	if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
		write(address, data);
		// TODO not actually done, need to write-back if eviction is required
		// Consider changing interface to detect conflict separately
		// (will also simplify reads considerably)
	}
}

L2Cache::outcome L2Cache::isPresent(int addressIn, int* addressOut) {
	int index = toIndex(addressIn);
	int tag = toTag(addressIn);
	if (ISA::isCodeAddress(addressIn)) {
		// Check instructions cache
		int blockWay0 = toInstructionsBlock(index, 0);
		int blockWay1 = toInstructionsBlock(index, 1);
		if ((instructionsValid[blockWay0] && (instructionsTag[blockWay0] == tag)) ||
			(instructionsValid[blockWay1] && (instructionsTag[blockWay1] == tag))) {
			// Mapped to block with matching tag
			return PRESENT;
		}
		if (!instructionsValid[blockWay0] || !instructionsValid[blockWay1]) {
			// Mapped to at least one invalid block
			return INVALID;
		}
		// Mapped to valid blocks with mismatching tag
		return CONFLICT;
	} else {
		// Check data cache
		int blockWay0 = toDataBlock(index, 0);
		int blockWay1 = toDataBlock(index, 1);
		if ((dataValid[blockWay0] && (dataTag[blockWay0] == tag)) ||
			(dataValid[blockWay1] && (dataTag[blockWay1] == tag))) {
			// Mapped to block with matching tag
			return PRESENT;
		}
		if (!dataValid[blockWay0] || !dataValid[blockWay1]) {
			// Mapped to at least one invalid block
			return INVALID;
		}
		// Mapped to valid blocks with mismatching tag
		return CONFLICT;
	}
}

int L2Cache::read(int address) {
	int index = toIndex(address);
	int tag = toTag(address);
	int offset = toOffset(address);

	if (ISA::isCodeAddress(address)) {
		// Use instructions buffer
		int blockWay0 = toInstructionsBlock(index, 0);
		int blockWay1 = toInstructionsBlock(index, 1);
		if (instructionsValid[blockWay0] && (instructionsTag[blockWay0] == tag)) {
			// Present in way 0
			instructionsWay0IsLru[index] = false;
			return *getInstructionPtr(index, 0, offset);
		} else {
			assert(instructionsValid[blockWay1])
			assert(instructionsTag[blockWay1] == tag);
			// Assume present in way 1
			instructionsWay0IsLru[index] = true;
			return *getInstructionPtr(index, 1, offset);
		}
	} else {
		// Use data buffer
		int blockWay0 = toDataBlock(index, 0);
		int blockWay1 = toDataBlock(index, 1);
		if (dataValid[blockWay0] && (dataTag[blockWay0] == tag)) {
			// Present in way 0
			dataWay0IsLru[index] = false;
			return *getDataPtr(index, 0, offset);
		} else {
			assert(dataValid[blockWay1] && (dataTag[blockWay1] == tag));
			// Assume present in way 1
			dataWay0IsLru[index] = true;
			return *getDataPtr(index, 1, offset);
		}
	}
}

void L2Cache::write(int address, int value, L2Cache::outcome expected_outcome) {
	int index = toIndex(address);
	int tag = toTag(address);
	int offset = toOffset(address);

	switch (expected_outcome) {
	case PRESENT:
		if (ISA::isCodeAddress(address)) {
			// Update block in instructions cache
			int blockWay0; = toInstructionsBlock(index, 0);
			int blockWay1; = toInstructionsBlock(index, 1);
			if (instructionsValid[blockWay0] && (instructionsTag[blockWay0] == tag)) {
				*getInstructionsPtr(index, 0, offset) = value;
				instructionsWay0IsLru[index] = false;
				return;
			} else {
				assert(instructionsValid[blockWay1]);
				assert(instructionsTag[blockWay1] == tag);
				// Assume block is present in other way
				*getInstructionsPtr(index, 1, offset) = value;
				instructionsWay0IsLru[index] = true;
				return;
			}
		} else {
			// Update block in data cache
			int blockWay0; = toDataBlock(index, 0);
			int blockWay1; = toDataBlock(index, 1);
			if (dataValid[blockWay0] && (dataTag[blockWay0] == tag)) {
				*getDataPtr(index, 0, offset) = value;
				dataWay0IsLru[index] = false;
				return;
			} else {
				assert(dataValid[blockWay1]);
				assert(dataTag[blockWay1] == tag);
				// Assume block is present in other way
				*getDataPtr(index, 1, offset) = value;
				dataWay0IsLru[index] = true;
				return;
			}
		}
	case CONFLICT:
		if (ISA::isCodeAddress(address)) {
			// Overwrite LRU block in instructions cache
			int wayLru;
			int blockWayLru;
			if (instructionsWay0IsLru[index]) {
				wayLru = 0;
				blockWayLru = toInstructionsBlock(index, 0);
				instructionsWay0IsLru[index] = false;
			} else {
				wayLru = 1;
				blockWayLru = toInstructionsBlock(index, 1);
				instructionsWay0IsLru[index] = true;
			}
			*getInstructionsPtr(index, wayLru, offset) = value;
			instructionsValid[blockWayLru] = true;
			instructionsTag[blockWayLru] = tag;
			return;
		} else {
			// Overwrite LRU block in data cache
			int wayLru;
			int blockWayLru;
			if (instructionsWay0IsLru[index]) {
				wayLru = 0;
				blockWayLru = toDataBlock(index, 0);
				instructionsWay0IsLru[index] = false;
			} else {
				wayLru = 1;
				blockWayLru = toDataBlock(index, 1);
				instructionsWay0IsLru[index] = true;
			}
			*getDataPtr(index, wayLru, offset) = value;
			dataValid[blockWayLru] = true;
			dataTag[blockWayLru] = tag;
			return;
		}
	case INVALID:
		// This is impossible since L1 is inclusive in L2 and L1 performs write-allocate
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
}

void L2Cache::evict(int address) {
	int index = toIndex(address);
	int tag = toTag(address);

	if (ISA::isCodeAddress(address)) {
		// Evict from instructions cache
		int blockWay0 = toWayInstruction(index, 0);
		int blockWay1 = toWayInstruction(index, 1);
		if (instructionsValid[blockWay0] && (instructionsTag[blockWay0] == tag)) {
			instructionsValid[blockWay0] = false;
		} else {
			assert(instructionsValid[blockWay1])
			assert(instructionsTag[blockWay1] == tag);
			// Assume eviction from the other way
			instructionsValid[blockWay1] = false;
		}
	} else {
		// Evict from data cache
		int blockWay0 = toWayData(index, 0);
		int blockWay1 = toWayData(index, 1);
		if (dataValid[blockWay0] && (dataTag[blockWay0] == tag)) {
			dataValid[blockWay0] = false;
		} else {
			assert(dataValid[blockWay1])
			assert(dataTag[blockWay1] == tag);
			// Assume eviction from the other way
			dataValid[blockWay1] = false;
		}
	}

	// Also tell L1 cache to evict if present (since it is inclusive in L2)
	if (l1Cache != NULL) {
		l1Cache->evict(address);
	}
}

int L2Cache::toTag(int address) {
	// Since there are two ways per block in the cache
	return address / (cacheSize / 4);
}

int L2Cache::toBlockNumber(int address) {
	// The cache is partitioned 50%/50% between instructions and data,
	// and is 2-way set associative.
	return (address / blockSize) % ((cacheSize / 4) / blockSize);// + ((cacheSize / 4) / blockSize * way);
}

int L2Cache::toAddress(int tag, int blockNumber, int blockOffset) {
	return (tag * (cacheSize / 4)) + (blockNumber * blockSize) + blockOffset;
}

int L2Cache::toWayInstruction(int blockNumber, int way) {
	// Ways are sequential in the instruction cache (first all of way 0, then all of way 1).
	return blockNumber + ((cacheSize / 4) / blockSize * way);
}

int L2Cache::toWayData(int blockNumber, int way) {
	// Ways are interleaved in the data cache (way 0, way 1, way 0, way 1...)
	return blockNumber + way;
}
