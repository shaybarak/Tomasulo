#include "L2Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

L2Cache::L2Cache(int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave,
		Cache* l1Cache)
		: Cache(blockSize, cacheSize, accessDelay, 2),
		  pL1Master(pL1Master), pRamSlave(pRamSlave),
		  l1Cache(l1Cache), state(READY), delay(-1) {
	// Also initialize dirty bits
	dirty.resize(valid.size());
	// Also initialize LRU bits (2 ways per block)
	way0IsLru.resize(valid.size() / 2);
}

void L2Cache::onTickUp(int now) {
	switch (state) {
	case READY:
		if (!pL1Master->masterValid) {
			break;
		}
		// Got request from CPU
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
		break;
	case WAIT_CWF:
	case WRITE_TO_RAM:
	default:
		// Unknown state
		assert(false);
		break;
	}
}

void L2Cache::onTickDown(int now) {
	switch (state) {

	}
}

bool L2Cache::isHit(int address) {

}

int L2Cache::read(int address) {

}

void L2Cache::write(int address, int value) {

}
