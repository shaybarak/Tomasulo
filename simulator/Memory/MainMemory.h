#pragma once

class MainMemory : Memory {
	MainMemory(int accessDelay) : Memory(NULL, SIZE, accessDelay) {}
	virtual bool read(int address, int* value);
	virtual bool write(int address, int value);

private:
	// Main memory is 16MB as defined in the exercise instructions
	static const int SIZE = 16000000;
};
