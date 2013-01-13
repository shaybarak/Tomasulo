#include "MainMemory.h"
#include "../MIPS32/ISA.h"

MainMemory::MainMemory(ISA::MemoryType memoryType, int accessDelay)
	: memoryType(memoryType), accessDelay(accessDelay) {
	buffer.resize(ISA::RAM_SIZE);
	words = (int*)&buffer[0];
}

int MainMemory::read(int address) {
	return words[address / sizeof(int)];
}
	
void MainMemory::write(int address, int value) {
	words[address / sizeof(int)] = value;
}
