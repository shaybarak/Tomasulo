#pragma once

#include "Memory.h"

class L1Cache : public Memory {
public:
	Cache(Memory* nextLevel, int sizeInBytes, int accessDelay, int sizeOfBlock);
	int getHitCount() { return hits; }
	int getMissCount() { return misses; }
	double getHitRate() { return (double)hits / (double)(hits + misses); }

private:
	// Invalid tag
	static const int INVALID = -1;
	// Cache line tags
	vector<short> tags;
	int sizeOfBlock;
	// Statistics
	int hits;
	int misses;
	// Next level of cache/memory
	Memory* nextLevel;
};
