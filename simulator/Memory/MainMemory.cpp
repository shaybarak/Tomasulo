#include "MainMemory.h"
#include "../MIPS32/ISA.h"

MainMemory::MainMemory(int accessDelay, int rowSize, MasterSlaveInterface* pL2Master, ISA::MemoryType memoryType)
	: accessDelay(accessDelay), rowSize(rowSize), pL2Master(pL2Master), memoryType(memoryType),
		  lastReadAddress(-1), busyReadingUntil(0) {
	buffer.resize(ISA::RAM_SIZE);
	words = (int*)&buffer[0];
}

vector<unsigned char>* MainMemory::getBuffer() { 
	return &buffer; 
}

void MainMemory::onTickUp(int now) {
	if (!pL2Master->slaveReady) {
		return;
	}
	if (!pL2Master->masterValid) {
		return;
	}
	if (openedRow == toRow(pL2Master->address)) {
		delayCountDown = 1;
	} else {
		delayCountDown = accessDelay;
	}
	pL2Master->slaveReady = false;
	pL2Master->slaveValid = false;
}

void MainMemory::onTickDown(int now) {
	if (!pL2Master->masterReady) {
		return;
	}
	if (--delayCountDown > 0) {
		return;
	}
	openedRow = toRow(pL2Master->address);
	if (pL2Master->writeEnable) {
		words[pL2Master->address / sizeof(int)] = pL2Master->data;
		pL2Master->slaveValid = false;
	} else {
		pL2Master->data = words[pL2Master->address / sizeof(int)];
		pL2Master->slaveValid = true;
	}
	pL2Master->slaveReady = true;
}

int MainMemory::toRow(int address) {
	return address / rowSize;
}
