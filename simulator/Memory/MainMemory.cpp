#include "MainMemory.h"
#include "../MIPS32/ISA.h"

MainMemory::MainMemory(int accessDelay, int rowSize)
		: accessDelay(accessDelay), rowSize(rowSize),
		  lastReadAddress(-1), busyReadingUntil(0) {
	buffer.resize(ISA::RAM_SIZE);
	words = (int*)&buffer[0];
	slaveReady = true;
}

vector<unsigned char>* MainMemory::getBuffer() { 
	return &buffer; 
}

void MainMemory::onTickUp(int now) {
	if (!slaveReady) {
		return;
	}
	if (!masterValid) {
		return;
	}
	if (openedRow == toRow(address)) {
		delayCountDown = 1;
	} else {
		delayCountDown = accessDelay;
	}
	slaveReady = false;
	slaveValid = false;
}

void MainMemory::onTickDown(int now) {
	delayCountDown--;
	if (!slaveReady) {
		return;
	}

	if (delayCountDown != 0) {
		slaveReady = false;
		slaveValid = false;
		return;
	}

	if (writeEnable) {
		words[address / sizeof(int)] = data;
		slaveValid = true;
	} else {
		data = words[address / sizeof(int)];
		slaveReady = true;
	}
}

int MainMemory::toRow(int address) {
	return address / rowSize;
}
