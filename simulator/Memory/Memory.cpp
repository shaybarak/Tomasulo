#include "Memory.h"
#include "../Output/HexDump.h"

Memory::Memory(Memory* nextLevel, int sizeInBytes, int accessDelay) {
	this.nextLevel = nextLevel;
	this.accessDelay = accessDelay;
	// Initialize memory to zeroes
	values.resize(sizeInBytes / sizeof(int), 0);
}

bool Memory::dump(FILE* out) {
	return HexDump::store
}
