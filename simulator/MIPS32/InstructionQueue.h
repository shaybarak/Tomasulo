#pragma once

#include "../Memory/MemorySystem.h"
#include "Future.h"
#include <queue>

using namespace std;

class InstructionQueue {
public:
	InstructionQueue(int depth, MemorySystem* instructionMemory)
		: depth(depth), instructionMemory(instructionMemory) {}
	/** Try to read a new instruction from the instruction memory. */
	bool tryReadNewInstruction(int now, int pc);
	/**
	 * Try to get the next instruction.
	 * Returns instruction index in program on success, negative on failure.
	 */
	int tryGetNextInstruction(int now);
	/** Returns the count of memory accesses made. */
	int getInstructionsReadCount() { return instructionsRead; }

private:
	MemorySystem* instructionMemory;
	int depth;
	queue<Future<int>> q;	
	int instructionsRead;
};
