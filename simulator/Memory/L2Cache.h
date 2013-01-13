#pragma once

#include "Cache.h"

class L2Cache : public Cache {
public:
	L2Cache(ISA::MemoryType memoryType, int blockSize, int cacheSize, int accessDelay, int l1BlockSize,
		MasterSlaveInterface* pL1Master, MasterSlaveInterface* pRamSlave);

	// Returns whether address is present in cache
	virtual bool isPresent(int address) = 0;

	/**
	 * Reads from cache.
	 * address: memory address to read from.
	 * May return uninitialized data. Precede with a call to isPresent.
	 */
	virtual int read(int address) = 0;
	
	/**
	 * Writes to cache.
	 * address: memory address to write to.
	 * data: memory value to write.
	 * way: destination way when relevant.
	 * May overwrite previous data.
	 */
	virtual void write(int address, int value, int way = 0);

private:
	vector<bool> dirty;
	vector<bool> way0IsLru;
};
