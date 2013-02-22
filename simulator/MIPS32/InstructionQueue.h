#pragma once

#include "../Memory/MemorySystem.h"
#include "Instruction.h"
#include "Future.h"
#include "ISA.h"
#include <queue>

using namespace std;

class InstructionQueue {
public:
	InstructionQueue(int depth, MemorySystem* instructionMemory, vector<Instruction*>* program1, vector<Instruction*>* program2) :
		depth(depth),
		instructionMemory(instructionMemory),
		program1(program1),
		program2(program2),
		pc(ISA::FIRST_PROGRAM_BASE),
		branched(false),
		executing1(true),
		halted2(false) {}
	/** Whether there are no additional instructions to execute. */
	bool isDone();
	/** Try to read a new instruction from the instruction memory. */
	bool tryFetch(int now);
	/**
	 * Try to get the next instruction to execute.
	 * Returns instruction on success, NULL on failure.
	 */
	Instruction* tryGetNextInstruction(int now);
	/** Pops the next instruction to execute (post-issue) */
	void popNextInstruction();
	/** Returns the current program counter */
	int getPc() { return pc; }
	/** Manually sets the PC (after branch execution). */
	void setPc(int newPc) { pc = newPc; branched = false; }

private:
	int pcToAddress(int pc);
	Instruction* getInstruction(int instructionINdex);
	void halt();

	int depth;
	MemorySystem* instructionMemory;
	vector<Instruction*>* program1;
	vector<Instruction*>* program2;
	queue<Future<Instruction*>> q;	
	int pc;
	bool branched;
	bool executing1;
	bool halted2;
};
