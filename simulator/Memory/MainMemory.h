#pragma once

class MainMemory : Clocked {
	MainMemory(char* memory, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel)
		: memory(memory), accessDelay(accessDelay), previousMemoryLevel(previousMemoryLevel) {}
	void read(int address, int* value);
	void write(int address, int value);

private:
	char* memory;
	int accessDelay;
	PreviousMemoryLevel* previousMemoryLevel;
};
