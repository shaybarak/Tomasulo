#pragma once

#include <vector>
#include "MemoryInterface.h"
#include "NextMemoryLevel.h"
#include "PreviousMemoryLevel.h"
using namespace std;

class MainMemory : public Clocked {
public:
	MainMemory(int accessDelay, int l2BlockSize, PreviousMemoryLevel* previousMemoryLevel);
	// WARNING: exposes non-const pointer to data member
	vector<unsigned char>* getBuffer();
	virtual void onTick(int now);
	int read(int offset);
	void write(int offset, int value);

private:
	vector<unsigned char> buffer;
	int* words;
	int accessDelay;
	int l2BlockSize;
	// For identifying sequential access
	int lastReadAddress;
	// Busy reading until this time
	int busyReadingUntil;
	PreviousMemoryLevel* previousMemoryLevel;
};
