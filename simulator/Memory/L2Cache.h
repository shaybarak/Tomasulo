#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int l1BlockSize,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave);

private:
	// Returns whether address is present in a given way
	bool isPresentInWay(int address, int way);
	
	// Returns whether address is a hit
	bool isHit(int address);
	
	// Returns vacant way (0 or 1) for address if exists or -1 otherwise
	int findVacancy(int address);

	/**
	 * Checks whether a block at an address is dirty and should be written back.
	 * Assumes that block is present.
	 */
	bool isDirty(int address);

	// Generates the next address in cyclic order
	int nextAddress(int address, int blockSize);

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
		READ_L1_BLOCK_FROM_RAM,	//reading rest of l1 block from main memory
		READ_REST_L2_BLOCK,		//reading rest of l2 block from memory
		WRITE_BACK,				//write l2 blck to ram, when write-back policy enforces
	};
	State state;

	vector<bool> dirty;
	vector<bool> way0IsLru;

	// Size of L1 block in bytes
	int l1BlockSize;

	// Words left until end of block
	int wordsLeft;

	// Copy of address for cyclic read/write
	int address;

	// Way to write into (kept between state transitions on read from RAM)
	int destinationWay;
};
