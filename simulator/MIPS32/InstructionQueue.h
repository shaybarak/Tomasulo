#pragma once

#include "../Memory/MemorySystem.h"
#include "Instruction.h"
#include "Future.h"
#include <queue>

using namespace std;

class InstructionQueue {
public:
	InstructionQueue(int depth, MemorySystem* instructionMemory, vector<Instruction*>* instructions)
		: depth(depth), instructionMemory(instructionMemory), instructions(instructions) {}
	/** Try to read a new instruction from the instruction memory. */
	bool tryReadNewInstruction(int now, int pc);
	/**
	 * Try to get the next instruction.
	 * Returns instruction in program on success, NULL on failure.
	 */
	Instruction* tryGetNextInstruction(int now);
	/** Returns the count of memory accesses made. */
	int getInstructionsReadCount() { return instructionsRead; }

private:
	int depth;
	MemorySystem* instructionMemory;
	vector<Instruction*>* instructions;
	queue<Future<Instruction*>> q;	
	int instructionsRead;
};
