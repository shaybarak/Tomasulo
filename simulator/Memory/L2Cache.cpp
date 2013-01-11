#include "L2Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

L2Cache::L2Cache(int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache)
		: Cache(blockSize, cacheSize, WAYS, accessDelay, previousMemoryLevel, nextMemoryLevel),
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
		pendingReadsInternal.erase(address);
		if (pendingReadsExternal.erase(address) > 0) {
			// Serve incoming data to lower level as well
			previousMemoryLevel->respondRead(address, data, now);
		}

		// Is this read response going to force eviction?
		outcome readOutcome = isPresent(address, &addressOut);
		switch (readOutcome) {
		PRESENT:
			// Filling a block brought from RAM (eviction has already occurred)
			map<int, int>::iterator writeBack = pendingWrites.find(address);
			// Perform write-back of overwritten value if necessary
			if (writeBack != pendingWrites.end()) {
				// Was waiting for this word for write-allocate, now send write
				address = writeBack->first;
				data = writeBack->second;
				write(address, data);
				nextMemoryLevel->requestWrite(address, data, now);
				pendingWrites.erase(writeBack);
			}
			write(address, data, PRESENT);
			break;
		INVALID:
			// First read from a block that was previously invalid
			write(address, data, INVALID);
			break;
		CONFLICT:
			// Critical word returned, met cache conflict
			if (isDirty(address)) {
				int oldData = read(addressOut);
				nextMemoryLevel->requestWrite(addressOut, oldData, now);
			}
			// Evict previous block contents
			evict(address);
			// Write critical word
			write(address, data, CONFLICT);
			break;
		default:
			assert(false);
			break;
		}
	}

	// Get write response from main memory
	else if (nextMemoryLevel->getWriteResponse(&address, now)) {
		// Do nothing (due to write-back policy)
	}

	// Get read request from L1 cache
	if (previousMemoryLevel->getReadRequest(&address, now)) {
		if ((pendingReadsInternal.find(address) != pendingReadsInternal.end()) ||
			(pendingReadsExternal.find(address) != pendingReadsExternal.end())) {
			// There is a pending read, so delay but mark this as a hit
			hits++;
			pendingReadsExternal.insert(address);
			// Do nothing (result will be sent back to L1 when the pending read returns)
		} else if (isPresent(address, &addressOut) == PRESENT) {
			// Can satisfy read from cache
			hits++;
			data = read(address);
			previousMemoryLevel->respondRead(address, data, now + accessDelay);
		} else {
			// Need to read from next level
			misses++;
			// Critical word first
			nextMemoryLevel->requestRead(address, now + accessDelay);
			pendingReadsExternal.insert(address);
			// Then critical L1 block
			int l1BlockSize = l1Cache->blockSize;
			int baseOfL1Block = address - (address % l1BlockSize);
			for (int i = 1; i < l1BlockSize / (int)sizeof(int); i++) {
				int fillAddress = baseOfL1Block + ((address + i * sizeof(int)) % l1BlockSize);
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
				data = read(fillAddress);
			}
			// Then rest of L2 block
			int baseOfL2Block = address - toOffset(address);
			for (int i = 1 + (l1BlockSize / (int)sizeof(int)); i < (blockSize / (int)sizeof(int)); i++) {
				int fillAddress = baseOfL2Block + ((address + i * sizeof(int)) % blockSize);
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
				data = read(fillAddress);
			}
		}
	}

	// Get write request from L1 cache
	else if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
		write(address, data);
		// TODO not actually done, need to write-back if eviction is required
		// Also need to update dirty bit
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

bool L2Cache::isDirty(int address) {
	int index = toIndex(address);

	if (ISA::isCodeAddress(address)) {
		// Check instructions cache
		int blockWay0 = toWayInstruction(index, 0);
		int blockWay1 = toWayInstruction(index, 1);
		if (instructionsValid[blockWay0] && (instructionsTag[blockWay0] == tag)) {
			return instructionsDirty[blockWay0];
		} else {
			assert(instructionsValid[blockWay1]);
			assert(instructionsTag[blockWay1] == tag);
			// Assume present in way 1
			return instructionsDirty[blockWay1];
		}
	} else {
		// Check data cache
		int blockWay0 = toWayData(index, 0);
		int blockWay1 = toWayData(index, 1);
		if (dataValid[blockWay0] && (dataTag[blockWay0] == tag)) {
			return dataDirty[blockWay0];
		} else {
			assert(dataValid[blockWay1]);
			assert(dataTag[blockWay1] == tag);
			// Assume present in way 1
			return dataDirty[blockWay1];
		}
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
	l1Cache->evict(address);
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
