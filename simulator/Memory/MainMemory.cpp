#include "MainMemory.h"
#include "../MIPS32/ISA.h"

int MainMemory::read(int address) {
	return words[address / sizeof(int)];
}
	
void MainMemory::write(int address, int value) {
	words[address / sizeof(int)] = value;
}
