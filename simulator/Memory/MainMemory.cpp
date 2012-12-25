#include "MainMemory.h"
#include "../MIPS32/ISA.h"

MainMemory::MainMemory(int accessDelay, PreviousMemoryLevel* previousMemoryLevel) 
	: accessDelay(accessDelay), previousMemoryLevel(previousMemoryLevel) {
	buffer(ISA::RAM_SIZE);
	words = &buffer[0];
}

int MainMemory::read(int offset) {
	return words[offset];
}

void MainMemory::write(int offset, int value) {
	words[offset] = value;
}

void MainMemory::onTick(int now) {

}
