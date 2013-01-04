#include "L2Cache.h"
#include "../MIPS32/ISA.h"

L2Cache::L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache)
		: Cache(buffer, blockSize, cacheSize, accessDelay, previousMemoryLevel, nextMemoryLevel)
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

	// Get read response from L2 cache
	if (nextMemoryLevel->getReadResponse(&address, &data, now)) {
		write(address, data);
		pendingReadsInternal.erase(address);
		if (pendingReadsExternal.erase(address) > 0) {
			// Serve incoming data to lower level
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
			for (int i = 1; i < blockSize / sizeof(int); i++) {
				int fillAddress = baseOfBlock + ((address + i * sizeof(int)) % blockSize);
				addressToEvict = 
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

bool L2Cache::read(int address, int* value, int* addressToEvict) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);

	if (ISA::isCodeAddress(address)) {
		// Use instructions buffer
		int way0 = toWayInstruction(blockNumber, 0);
		int way1 = toWayInstruction(blockNumber, 1);
		if (instructionsValid[way0] && (instructionsTag[way0] == toTag(address))) {
			// Present in way 0
			*value = *getInstructionsPtr(way0, address % blockSize);
			instructionsWay0IsLru[blockNumber] = false;
			return true;
		} else if (instructionsValid[way1] && (instructionsTag[way1] == toTag(address))) {
			// Present in way 1
			*value = *getInstructionsPtr(way1, address % blockSize);
			instructionsWay0IsLru[blockNumber] = true;
			return true;
		} else {
			// Not present
			// Return value in LRU way (will be written back to next level on eviction)
			int lruWay;
			if (instructionsWay0IsLru[blockNumber]) {
				lruWay = way0;
			} else {
				lruWay = way1;
			}
			*value = *getInstructionsPtr(lruWay, address % blockSize);
			if (instructionsValid[lruWay]) {
				*addressToEvict = toAddress(instructionsTag[lruWay], blockNumber, address % blockSize);
			} else {
				*addressToEvict = -1;
			}
			return false;
		}
	} else {
		// Use data buffer
		int way0 = toWayData(blockNumber, 0);
		int way1 = toWayData(blockNumber, 1);
		if (dataValid[way0] && (dataTag[way0] == tag)) {
			// Present in way 0
			*value = *getDataPtr(way0, address % blockSize);
			dataWay0IsLru[blockNumber] = false;
			return true;
		} else if (dataValid[way1] && (dataTag[way1] == tag)) {
			// Present in way 1
			*value = *getDataPtr(way1, address % blockSize);
			dataWay0IsLru[blockNumber] = true;
			return true;
		} else {
			// Not present
			// Return value in LRU way (will be written back to next level on eviction)
			int lruWay;
			if (dataWay0IsLru[blockNumber]) {
				lruWay = way0;
			} else {
				lruWay = way1;
			}
			*value = *getDataPtr(lruWay, address % blockSize);
			if (dataValid[lruWay]) {
				*addressToEvict = toAddress(dataTag[lruWay], blockNumber, address % blockSize);
			} else {
				*addressToEvict = -1;
			}
			return false;
		}
	}
}

void L2Cache::write(int address, int value) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);

	if (ISA::isCodeAddress(address)) {
		// Use instructions cache
		int way;
		if (instructionsWay0IsLru[blockNumber]) {
			// Way 0 is the LRU
			way = toWayInstruction(blockNumber, 0);
		} else {
			// Way 1 is the LRU
			way = toWayInstruction(blockNumber, 1);
		}
		*toInstructionsPtr(way, address % blockSize) = value;
		if (instructionsTag[way] != tag) {
			// Need to evict old block
			evict(address);
		}
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
		*toDataPtr(way, address % blockSize) = value;
		if (dataTag[way] != tag) {
			// Need to evict old block
			evict(address);
		}
		dataTag[way] = tag;
		dataValid[way] = true;
	}
}

bool L2Cache::evict(int address) {
	int blockNumber = toBlockNumber(address);
	int tag = toTag(address);
	bool evicted = false;

	if (ISA::isCodeAddress(address)) {
		// Evict from instructions cache if present
		int way0 = toWayInstruction(blockNumber, 0);
		int way1 = toWayInstruction(blockNumber, 1);
		// Look for a match in both ways
		if (instructionsTag[way0] == tag) {
			instructionsValid[way0] = false;
			evicted = true;
		} else if (instructionsTag[way1] == tag) {
			isntructionsValid[way1] = false;
			evicted = true;
		}
	} else {
		// Evict from data cache if present
		int way0 = toWayData(blockNumber, 0);
		int way1 = toWayData(blockNumber, 1);
		// Look for a match in both ways
		if (dataTag[way0] == tag) {
			dataValid[way0] = false;
			evicted = true;
		} else if (dataTag[way1] == tag) {
			dataValid[way1] = false;
			evicted = true;
		}
	}

	// Also tell L1 cache to evict if present (since it is inclusive in L2)
	if (l1Cache != NULL) {
		l1Cache->evict(address);
	}

	return evicted;
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
