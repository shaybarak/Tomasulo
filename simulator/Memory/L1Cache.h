#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pCpuMaster, MasterSlaveInterface* pL2Slave,
		L2Cache* l2Cache)
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

private:
	// Returns whether address is a hit
	bool isHit(int address);

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data, users should precede with a call to isPresent.
	 */
	int read(int address);
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * data: memory value to write.
	 * May overwrite previous data, users should precede with a call to isPresent.
	 */
	void write(int address, int value);

	/**
	 * Instructs cache to invalidate a block by address.
	 * Used by L2 cache to maintain inclusivity of L1.
	 * On hit invalidates and returns true, on miss returns false.
	 */
	bool invalidate(int address);
	
	static const int WAYS = 1;

	MasterSlaveInterface* pCpuMaster;
	MasterSlaveInterface* pL2Slave;

	// Access delay time remaining
	int delay;

	// Access to L2 cache for forwarding writes
	L2Cache* l2Cache;

	enum State {
		READY,		// Ready for next request from master (initial state)
		ACCESSING,	// Waiting on access delay
		READ_MISS,	// Read miss
		WAIT_CWF,	// Waiting for critical word first
		WAIT_REST,	// Waiting for rest of block
		WRITE_MISS,	// Write miss
		WAIT_WA,	// Waiting for write-allocate
	};
	State state;
};
