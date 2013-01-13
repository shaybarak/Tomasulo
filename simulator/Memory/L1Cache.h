#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pCpuMaster, MasterSlaveInterface* pL2Slave)
		: Cache(memoryType, blockSize, cacheSize, accessDelay), 
		pCpuMaster(pCpuMaster), pL2Slave(pL2Slave), state(READY), delay(-1) {}

private:
	MasterSlaveInterface* pCpuMaster;
	MasterSlaveInterface* pL2Slave;

	// Access delay time remaining
	int delay;

	// Access to L2 cache for forwarding writes
	Cache* l2Cache;

	enum State {
		READY,		// Ready for next request from master (initial state)
		ACCESSING,	// Waiting on access delay
		READ_MISS,	// Read miss
		WAIT_CWF,	// Waiting for critical word first
		WRITE_MISS,	// Write miss
		WAIT_WA,	// Waiting for write-allocate
	};
	State state;
};
