#pragma once

#include "L1Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int blockSize, int cacheSize, int accessDelay,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave,
		L1Cache* l1Cache);
	// For reading signals from L1 cache and sending signals to RAM
	virtual void onTickUp(int now);
	// For reading signals from RAM and sending signals to L1 cache
	virtual void onTickDown(int now);

	// L1Cache is allowed private access to L2Cache
	// (since L1 is inclusive in L2, L1 must have intimate knowledge of L2)
	friend class L1Cache;

protected:
	typedef Cache::outcome outcome;
	virtual outcome isPresent(int addressIn, int* addressOut);
	virtual void evict(int address);

private:
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
	 * expected_outcome: obtained from isPresent, used to determine write policy.
	 * May overwrite previous data, users should precede with a call to isPresent.
	 */
	void write(int address, int data, outcome expected_outcome);

	/**
	 * Checks whether a block at an address is dirty and should be written back.
	 * Assumes that block is present, users should precede with a call to isPresent.
	 */
	bool isDirty(int address);

private:
	enum L2CacheState {
		READY,					//ready to serve
		ACCESSING,				//trying to access a word inside the cache
		SERVE_REST_L1_BLOCK,	//sending rest of l1 block, after critical word
		WAIT_CWF,				//waiting for main memory to return CriticalWordFirst
		READ_REST_L1_BLOCK,		//reading rest of l1 block from main memory
		WRITE_TO_RAM,		//write l2 blck to ram, when write-back policy enforces
		READ_REST_L2_BLOCK,		//reading rest of l2 block from memory
	};
	static const int WAYS = 2;

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;

	// Associated L1 cache
	L1Cache* l1Cache;

	//internal state of cache
	L2CacheState state;

	MasterSlaveInterface* pL1Master;
	MasterSlaveInterface* pRamSlave;

	int delayCountDown;

};
