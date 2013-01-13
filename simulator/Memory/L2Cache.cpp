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

void L2Cache::setL1Cache(Cache* l1Cache) {
	this->l1Cache = l1Cache;
}

void L2Cache::onTickUp(int now) {
	switch (state) {
	case READY:
		if (!pL1Master->masterValid) {
			break;
		}
		// Got request from L1 cache
		if (pL1Master->writeEnable) {
			// Not expecting writes from L1 via bus interface but through inclusive write shortcut
			assert(false);
		}
		delay = accessDelay;
		pL1Master->slaveReady = false;
		pL1Master->slaveValid = false;
		// Accessing internal buffer
		state = ACCESSING;
		break;
	
	case ACCESSING:
		// Blocked on internal buffers
		break;
	
	case SERVE_REST_L1_BLOCK:
		// Do nothing
		break;
	
	// Behavior on tickUp is identical
	case READ_L1_BLOCK_FROM_RAM:
	case READ_REST_L2_BLOCK:
		if (!pRamSlave->slaveReady) {
			// Blocking on RAM
			break;
		}
		// Read next word from RAM
		pRamSlave->address = address;
		pRamSlave->writeEnable = false;
		pRamSlave->masterValid = true;
		break;
	
	case WRITE_BACK:
		if (!pRamSlave->slaveReady) {
			// Blocking on RAM
			break;
		}
		// Write back next word to RAM
		pRamSlave->address = address;
		pRamSlave->data = read(address);
		pRamSlave->writeEnable = true;
		pRamSlave->masterValid;
		break;
	
	default:
		// Unknown state
		assert(false);
		break;
	}
}

void L2Cache::onTickDown(int now) {
	switch (state) {
	case READY:
		// Nothing to do
		break;
	
	case ACCESSING:
		delay--;
		if (delay > 0) {
			break;
		}
		assert(delay >= 0);
		if (isHit(pL1Master->address)) {
			// Hit
			hits++;
			// Critical word first
			pL1Master->data = read(pL1Master->address);
			pL1Master->slaveValid = true;
			wordsLeft = l1BlockSize / sizeof(int) - 1;
			state = SERVE_REST_L1_BLOCK;
		} else {
			// Miss
			misses++;
			destinationWay = findVacancy(pL1Master->address);
			if (destinationWay >= 0) {
				// There is a vacant spot under this index, use it
				address = pL1Master->address;
				wordsLeft = l1BlockSize / sizeof(int);
				state = READ_L1_BLOCK_FROM_RAM;
			} else {
				// Must evict LRU block in index to bring in new block from RAM
				if (way0IsLru[toIndex(pL1Master->address)]) {
					destinationWay = 0;
				} else {
					destinationWay = 1;
				}
				if (dirty[toBlock(toIndex(pL1Master->address), destinationWay)]) {
					// Need to write back before bringing in new block
					state = WRITE_BACK;
					wordsLeft = blockSize / sizeof(int);
					// Start write back at start of L2 block
					address = (pL1Master->address / blockSize) * blockSize;
				} else {
					address = pL1Master->address;
					wordsLeft = l1BlockSize / sizeof(int);
					state = READ_L1_BLOCK_FROM_RAM;
				}
			}
		}
		break;
	
	case SERVE_REST_L1_BLOCK:
		pL1Master->address = nextAddress(pL1Master->address, l1BlockSize);
		pL1Master->data = read(pL1Master->address);
		pL1Master->slaveValid = true;
		wordsLeft--;
		if (wordsLeft == 0) {
			// Returned last word in L1 block
			pL1Master->slaveReady = true;
			state = READY;
			break;
		}
		assert(wordsLeft >= 0);
		break;
	
	case READ_L1_BLOCK_FROM_RAM:
		if (!pRamSlave->slaveValid) {
			// Blocking on RAM
			break;
		}
		// Store result
		write(pRamSlave->address, pRamSlave->data, destinationWay);
		// Evict conflicting block from L1
		l1Cache->invalidate(address);
		// Forward to L1
		pL1Master->address = pRamSlave->address;
		pL1Master->data = pRamSlave->data;
		pL1Master->slaveValid = true;
		// Prepare to handle next word in L1 block
		address = nextAddress(address, l1BlockSize);
		wordsLeft--;
		if (wordsLeft == 0) {
			wordsLeft = (blockSize - l1BlockSize) / sizeof(int);
			address = (address / blockSize * blockSize) + ((address / l1BlockSize * l1BlockSize + l1BlockSize) % blockSize);
			state = READ_REST_L2_BLOCK;
		}
		assert(wordsLeft >= 0);
		break;
	
	case READ_REST_L2_BLOCK:
		if (!pRamSlave->slaveValid) {
			// Blocking on RAM
			break;
		}
		// Store result
		write(pRamSlave->address, pRamSlave->data, destinationWay);
		// Evict conflicting block from L1
		l1Cache->invalidate(address);
		// Prepare to handle next word in L2 block
		address = nextAddress(address, blockSize);
		wordsLeft--;
		if (wordsLeft == 0) {
			pRamSlave->masterValid = false;
			pL1Master->slaveReady = true;
			state = READY;
		}
		assert(wordsLeft >= 0);
		break;
	
	case WRITE_BACK:
		if (!pRamSlave->slaveValid) {
			// Blocking on RAM
			break;
		}
		address += sizeof(int);
		wordsLeft--;
		if (wordsLeft == 0) {
			state = READ_L1_BLOCK_FROM_RAM;
			wordsLeft = l1BlockSize / sizeof(int);
		}
		assert(wordsLeft >= 0);
		break;
	
	default:
		// Unknown state
		assert(false);
		break;
	}
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
