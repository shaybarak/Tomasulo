#include "L1Cache.h"
#include "../MIPS32/ISA.h"

void L1Cache::onTick(int now) {
	// TODO
}

bool L1Cache::read(int address, int* value) {
	if (pendingReads.find(address) != pendingReads.end()) {
		// There is a pending read to this address, hence it's invalid
		return false;
	}
	if (address < ISA::CODE_BASE) {
		// Use instructions buffer
		*value = instructions[address / sizeof(int)]
		// Verify valid & tag
		return instructionsValid[address / blockSize]
			&& (toTag(address) == instructionsTag[address / blockSize]);
	} else {
		*value = data[(address / sizeof(int)]
		// Verify valid & tag
		return dataValid[address / blockSize]
			&& (toTag(address) == dataTag[address / blockSize]);
	}
}

void L1Cache::write(int address, int value) {
	if (address < ISA::CODE_BASE) {
		// Use instructions buffer
		instructions[address / sizeof(int)] = value;
		instructionsTag[address / blockSize] = toTag(address);
		instructionsValid[address / blockSize] = true;
	} else {
		data[address / sizeof(int)] = value;
		dataTag[address / blockSize] = toTag(address);
		dataValid[address / blockSize] = true;
	}
	// If address was pending a read, no need to wait
	pendingReads.erase(address);
}
