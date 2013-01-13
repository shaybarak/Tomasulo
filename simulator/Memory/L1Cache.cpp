#include "L1Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

void L1Cache::setL2Cache(Cache* l2Cache) {
	this->l2Cache = l2Cache;
}

void L1Cache::onTickUp(int now) {
	switch (state) {
	case READY:
		if (!pCpuMaster->masterValid) {
			break;
		}
		// Got request from CPU
		delay = accessDelay;
		pCpuMaster->slaveReady = false;
		pCpuMaster->slaveValid = false;
		// Accessing internal buffer
		state = ACCESSING;
		break;
	
	case ACCESSING:
		// Blocked on internal buffers
		break;
	
	case READ_MISS:
		// Read miss - bring entire L1 block from L2
		if (!pL2Slave->slaveReady) {
			// Wait for L2 to become ready
			break;
		}
		// Request critical word first
		pL2Slave->address = pCpuMaster->address;
		pL2Slave->writeEnable = false;
		pL2Slave->masterValid = true;
		state = WAIT_CWF;
		break;
	
	case WAIT_CWF:
		// Blocked on L2 cache
		break;
	
	case WRITE_MISS:
		// Write allocate: first bring entire L1 block from L2
		if (!pL2Slave->slaveReady) {
			// Wait for L2 to become ready
			break;
		}
		// Start reading at base of block
		pL2Slave->address = pCpuMaster->address - toOffset(pCpuMaster->address);
		pL2Slave->writeEnable = false;
		pL2Slave->masterValid = true;
		state = WAIT_WA;
		break;
	
	case WAIT_WA:
		break;
	
	default:
		// Unknown state
		assert(false);
		break;
	}
}

void L1Cache::onTickDown(int now) {
	switch (state) {
	case READY:
		// Nothing to do
		break;
	
	case ACCESSING:
		delay--;
		if (delay > 0) {
			break;
		}
		if (isHit(pCpuMaster->address)) {
			// Hit
			hits++;
			if (!pCpuMaster->writeEnable) {
				// Return cached data on read hit
				pCpuMaster->data = read(pCpuMaster->address);
			} else {
				// Commit write on write hit
				write(pCpuMaster->address, pCpuMaster->data);
			}
			pCpuMaster->slaveValid = true;
			pCpuMaster->slaveReady = true;
			state = READY;
		} else {
			// Miss
			misses++;
			if (!pCpuMaster->writeEnable) {
				state = READ_MISS;
			} else {
				state = WRITE_MISS;
			}
		}
		break;
	
	case READ_MISS:
		// Unexpected state
		assert(false);
		break;
	
	case WAIT_CWF:
		if (!pL2Slave->slaveValid) {
			// Still blocking on L2
			break;
		}
		// Early restart
		write(pCpuMaster->address, pL2Slave->data);
		pCpuMaster->data = pL2Slave->data;
		pCpuMaster->slaveValid = true;
		state = WAIT_WA;
		break;
	
	case WRITE_MISS:
		// Unexpected state
		assert(false);
		break;
	
	case WAIT_WA:
		pCpuMaster->slaveValid = false;
		write(pL2Slave->address, pL2Slave->data);
		if (pL2Slave->slaveReady) {
			// Done filling
			pL2Slave->masterValid = false;
			pCpuMaster->slaveReady = true;
			state = READY;
		}
		break;
	
	default:
		// Unknown state
		assert(false);
		break;
	}
}

bool L1Cache::isHit(int address) {
	int index = toIndex(address);
	int tag = toTag(address);
	return (valid[index] && (tags[index] == tag));
}

int L1Cache::read(int address) {
	int index = toIndex(address);
	int offset = toOffset(address);
	return *getWordPtr(index, offset);
}

void L1Cache::write(int address, int value) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	*getWordPtr(index, offset) = value;
	valid[index] = true;
	tags[index] = tag;
	if (l2Cache != NULL) {
		l2Cache->write(address, value);
	}
}

bool L1Cache::invalidate(int address) {
	if (isHit(address)) {
		valid[toIndex(address)] = false;
		return true;
	}
	return false;
}
