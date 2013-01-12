#pragma once

#include <vector>
#include "../Clock/Clocked.h"
#include "MasterSlaveInterface.h"
using namespace std;

class MainMemory : public Clocked {
public:
	MainMemory(int accessDelay, int rowSize, MasterSlaveInterface* pL2Master);
	// Design flaw: exposes non-const pointer to data member
	// (required for loading memory initialization)
	vector<unsigned char>* getBuffer();
	// For reading signals from L2 cache
	virtual void onTickUp(int now);
	// For sending signals to L2 cache
	virtual void onTickDown(int now);

	//interface for communication with L2
	MasterSlaveInterface* pL2Master;

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
