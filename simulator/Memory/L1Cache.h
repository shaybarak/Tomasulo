#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	L1Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel)
		: Cache(buffer, blockSize, cacheSize, WAYS, accessDelay, previousMemoryLevel, nextMemoryLevel) {}
	virtual void onTick(int now);

	// L2Cache is allowed private access to L1Cache
	friend class L2Cache;

protected:
	typedef Cache::outcome outcome;
	virtual outcome isPresent(int addressIn, int* addressOut);
	virtual int read(int address);
	virtual void write(int address, int value);
	virtual void evict(int address);

private:
	static const int WAYS = 1;
};
