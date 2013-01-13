#pragma once

#include <vector>
#include "../MIPS32/ISA.h"
#include "../Clock/Clocked.h"
#include "MasterSlaveInterface.h"
using namespace std;

class MainMemory : public Clocked {
public:
	MainMemory(int accessDelay, int rowSize, MasterSlaveInterface* pL2Master, ISA::MemoryType memoryType);
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
	// Defines whether type is instructions/data
	ISA::MemoryType memoryType;

	vector<unsigned char> buffer;
	int* words;
	int accessDelay;
	int delay; 
	int rowSize;
	// For identifying sequential access
	int openRow;
	// Busy reading until this time
	int busyReadingUntil;

	int toRow(int address);

	int read(int offset);
	void write(int offset, int value);
	bool ready;
};
