#pragma once

#include "Cache.h"
#include "L1Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache);
	virtual void onTickDown(int now);
	virtual void onTickUp(int now);

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
	static const int WAYS = 2;

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;

	// Associated L1 cache
	L1Cache* l1Cache;
};
