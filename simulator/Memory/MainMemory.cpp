#include "MainMemory.h"
#include "../MIPS32/ISA.h"

MainMemory::MainMemory(int accessDelay, int l2BlockSize, PreviousMemoryLevel* previousMemoryLevel) 
		: accessDelay(accessDelay), l2BlockSize(l2BlockSize), previousMemoryLevel(previousMemoryLevel),
		  lastReadAddress(-1), busyReadingUntil(0) {
	buffer.resize(ISA::RAM_SIZE);
	words = (int*)&buffer[0];
}

vector<unsigned char>* MainMemory::getBuffer() { 
	return &buffer; 
}

void MainMemory::onTick(int now) {
	int address, data;
	
	// Serve read request
	if ((now >= busyReadingUntil) && previousMemoryLevel->getReadRequest(&address, now)) {
		// Simulate delay
		int delay;
		if (address % l2BlockSize == lastReadAddress % l2BlockSize) {
			// Sequential read
			delay = 1;
		} else {
			// Non-sequential read
			delay = accessDelay;
		}

		previousMemoryLevel->respondRead(address, words[address / sizeof(int)], now + delay);
		busyReadingUntil = now + delay;
		lastReadAddress = address;
	}
	
	// Serve write request
	if (previousMemoryLevel->getWriteRequest(&address, &data, now)) {
		// Assume no write delay
		words[address / sizeof(int)] = data;
	}
}
