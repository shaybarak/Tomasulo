#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave,
		Cache* l1Cache);
	// For reading signals from L1 cache and sending signals to RAM
	virtual void onTickUp(int now);
	// For reading signals from RAM and sending signals to L1 cache
	virtual void onTickDown(int now);

protected:
	virtual void write(int address, int value);
	virtual bool invalidate(int address) { return false; }  // Not implemented in L2

private:
	// Returns whether address is present in a given way
	bool isPresentInWay(int address, int way);
	// Returns whether address is a hit
	bool isHit(int address);

	/**
	 * Checks whether a block at an address is dirty and should be written back.
	 * Assumes that block is present.
	 */
	bool isDirty(int address);

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data, users should precede with a call to isPresent.
	 */
	int read(int address);

	MasterSlaveInterface* pL1Master;
	MasterSlaveInterface* pRamSlave;

	// Access delay time remaining
	int delay;

	// Access to L1 cache for invoking invalidations on L2 eviction
	Cache* l1Cache;

	enum State {
		READY,					//ready to serve
		ACCESSING,				//trying to access a word inside the cache
		SERVE_REST_L1_BLOCK,	//sending rest of l1 block, after critical word
		WAIT_CWF,				//waiting for main memory to return CriticalWordFirst
		READ_REST_L1_BLOCK,		//reading rest of l1 block from main memory
		WRITE_TO_RAM,			//write l2 blck to ram, when write-back policy enforces
		READ_REST_L2_BLOCK,		//reading rest of l2 block from memory
	};
	State state;

	vector<bool> dirty;
	vector<bool> way0IsLru;
};
