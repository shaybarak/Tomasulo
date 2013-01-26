#pragma once

#include "../Memory/MemorySystem.h"
#include "Instruction.h"
#include "Future.h"
#include <queue>

using namespace std;

class InstructionQueue {
public:
	InstructionQueue(int depth, MemorySystem* instructionMemory, vector<Instruction*>* instructions) :
		depth(depth),
		instructionMemory(instructionMemory),
		instructions(instructions),
		pc(0),
		branched(false),
		halted(false) {}
	/** Try to read a new instruction from the instruction memory. */
	bool tryReadNewInstruction(int now);
	/**
	 * Try to get the next instruction.
	 * Returns instruction in program on success, NULL on failure.
	 */
	Instruction* tryGetNextInstruction(int now);
	/** Returns the current program counter */
	int getPc() { return pc; }
	/** Manually sets the PC (after branch execution). */
	void setPc(int newPc) { pc = newPc; branched = false; }

private:
	int depth;
	MemorySystem* instructionMemory;
	vector<Instruction*>* instructions;
	queue<Future<Instruction*>> q;	
	int pc;
	bool branched;
	bool halted;
};
