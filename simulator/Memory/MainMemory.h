#pragma once

#include <vector>
#include "MemoryInterface.h"
#include "NextMemoryLevel.h"
#include "PreviousMemoryLevel.h"
#include "../Clock/Clocked.h"
#include "MasterSlaveInterface.h"
using namespace std;

class MainMemory : public Clocked, public MasterSlaveInterface {
public:
	MainMemory(int accessDelay, int rowSize);
	// Design flaw: exposes non-const pointer to data member
	vector<unsigned char>* getBuffer();
	virtual void onTickUp(int now);
	virtual void onTickDown(int now);

private:
	vector<unsigned char> buffer;
	int* words;
	int accessDelay;
	//will be used as a down counter for serving the request
	int delayCountDown; 
	int rowSize;
	int openedRow;
	// For identifying sequential access
	int lastReadAddress;
	// Busy reading until this time
	int busyReadingUntil;
	
	int toRow(int address);

	int read(int offset);
	void write(int offset, int value);
	bool ready;
};
