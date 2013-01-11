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
	if (openedRow == getAddressRow(address)) {
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
	if (delayCountDown == 0) {
		if (writeEnable) {
			words[address / sizeof(int)] = data;
			slaveValid = true;
		} else {
			data = words[address / sizeof(int)];
			slaveReady = true;
		}
	} else {
		slaveReady = false;
		slaveValid = false;
	}
}

int MainMemory::getAddressRow(int address) {
	return address / rowSize;
}

//void MainMemory::onTick(int now) {
//	int address, data;
//	
//	// Serve read request
//	if ((now >= busyReadingUntil) && previousMemoryLevel->getReadRequest(&address, now)) {
//		// Simulate delay
//		int delay;
//		if (address / rowSize == lastReadAddress / rowSize) {
//			// Sequential read
//			delay = 1;
//		} else {
//			// Non-sequential read
//			delay = accessDelay;
//		}
//
//		previousMemoryLevel->respondRead(address, words[address / sizeof(int)], now + delay);
//		busyReadingUntil = now + delay;
//		lastReadAddress = address;
//	}
//	
//	// Serve write request
//	if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
//		// Assume no write delay
//		words[address / sizeof(int)] = data;
//	}
//}
