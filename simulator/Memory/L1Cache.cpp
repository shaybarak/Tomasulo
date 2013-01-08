#include "L1Cache.h"
#include "../MIPS32/ISA.h"

void L1Cache::onTick(int now) {
	int address, data, addressOut;

	// Get read response from L2 cache
	if (nextMemoryLevel->getReadResponse(&address, &data, now)) {
		write(address, data);
		pendingReadsInternal.erase(address);
		if (pendingReadsExternal.erase(address) > 0) {
			// Serve incoming data to lower level
			previousMemoryLevel->respondRead(address, data, now);
		}
		
		map<int, int>::iterator writeAllocate = pendingWrites.find(address);
		if (writeAllocate != pendingWrites.end()) {
			// Was waiting for this word for write-allocate, now send write
			address = writeAllocate->first;
			data = writeAllocate->second;
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
		} else if (isPresent(address, &addressOut) == PRESENT) {
			// Can satisfy read from cache
			data = read(address);
			hits++;
			previousMemoryLevel->respondRead(address, data, now + accessDelay);
		} else {
			// Need to read from next level
			misses++;
			// Critical word first
			nextMemoryLevel->requestRead(address, now + accessDelay);
			pendingReadsExternal.insert(address);
			// Read rest of block
			int baseOfBlock = address - toOffset(address);
			for (int i = 1; i < blockSize / (int)sizeof(int); i++) {
				int fillAddress = baseOfBlock + toOffset(address + i * sizeof(int));
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
			pendingWrites[address] = data;
		} else if (isPresent(address, &addressOut) == PRESENT) {
			// No need to write-allocate
			hits++;
			write(address, data);
			nextMemoryLevel->requestWrite(address, data, now + accessDelay);
		} else {
			// Need to read from next level for write-allocate
			misses++;
			// Critical word first
			int baseOfBlock = address - toOffset(address);
			for (int i = 0; i < blockSize / (int)sizeof(int); i++) {
				int fillAddress = baseOfBlock + toOffset(address + i * sizeof(int));
				nextMemoryLevel->requestRead(fillAddress, now + accessDelay + i);
				pendingReadsInternal.insert(fillAddress);
			}
			// Write critical word first
			pendingWrites[address] = data;
		}
	}
}

L1Cache::outcome L1Cache::isPresent(int addressIn, int* addressOut) {
	int index = toIndex(addressIn);
	int tag = toTag(addressIn);
	if (ISA::isCodeAddress(addressIn)) {
		// Check instructions cache
		if (!instructionsValid[index]) {
			// Mapped to invalid block
			return INVALID;
		}
		if (instructionsTag[index] == tag) {
			// Mapped to block with matching tag
			return PRESENT;
		} else {
			// Mapped to block with mismatching tag
			return CONFLICT;
		}
	} else {
		// Check data cache
		if (!dataValid[index]) {
			// Mapped to invalid block
			return INVALID;
		}
		if (dataTag[index] == tag) {
			// Mapped to block with matching tag
			return PRESENT;
		} else {
			// Mapped to block with mismatching tag
			return CONFLICT;
		}
	}
}

int L1Cache::read(int address) {
	int index = toIndex(address);
	int offset = toOffset(address);
	if (ISA::isCodeAddress(address)) {
		// Read from instructions cache
		return *getInstructionPtr(index, offset);
	} else {
		// Read from data cache
		return *getDataPtr(index, offset);
	}
}

void L1Cache::write(int address, int value) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	if (ISA::isCodeAddress(address)) {
		// Write to instructions cache
		*getInstructionPtr(index, offset) = value;
		instructionsValid[index] = true;
		instructionsTag[index] = tag;
	} else {
		// Write to data cache
		*getDataPtr(index, offset) = value;
		dataValid[index] = true;
		dataTag[index] = tag;
	}
}

void L1Cache::evict(int address) {
	int index = toIndex(address);
	if (ISA::isCodeAddress(address)) {
		// Invalidate in instructions cache
		instructionsValid[index] = false;
	} else {
		// Invalidate in data cache
		dataValid[index] = false;
	}
}
