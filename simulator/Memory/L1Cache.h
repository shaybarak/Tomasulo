#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pCpuMaster, MasterSlaveInterface* pL2Slave,
		Cache* l2Cache)
		: Cache(blockSize, cacheSize, accessDelay),
		  pCpuMaster(pCpuMaster), pL2Slave(pL2Slave),
		  l2Cache(l2Cache), state(READY), delay(-1) {}
	// For reading signals from CPU and sending signals to L2 cache
	virtual void onTickUp(int now);
	// For reading signals from L2 cache and sending signals to CPU
	virtual void onTickDown(int now);

	// L2Cache is allowed private access to L1Cache
	// (since L1 is inclusive in L2, L2 must have intimate knowledge of L1)
	friend class L2Cache;


protected:
	virtual void write(int address, int value);
	virtual bool invalidate(int address);

private:
	// Returns whether address is a hit
	bool isHit(int address);

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data.
	 */
	int read(int address);

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
