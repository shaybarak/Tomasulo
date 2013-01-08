#pragma once

#include "Cache.h"
#include "L1Cache.h"
#include "../Clock/Clocked.h"

class L2Cache : public Cache, public Clocked {
public:
	L2Cache(int* buffer, int blockSize, int cacheSize, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel, NextMemoryLevel* nextMemoryLevel,
		L1Cache* l1Cache);
	virtual void onTick(int now);

protected:
	typedef Cache::outcome outcome;
	virtual outcome isPresent(int addressIn, int* addressOut);
	virtual int read(int address);
	virtual void write(int address, int data);
	virtual void evict(int address);

private:
	static const int WAYS = 2;

	vector<bool> instructionsDirty;
	vector<bool> dataDirty;
	vector<bool> instructionsWay0IsLru;
	vector<bool> dataWay0IsLru;

	// Associated L1 cache
	L1Cache* l1Cache;
};
