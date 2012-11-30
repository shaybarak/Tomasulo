#pragma once

#include "ISA.h"
#include "Instruction.h"
#include <vector>
using namespace std;

/**
 * MIPS32 CPU.
 * No pipelining.
 * Single memory level (no caches).
 */
class CPU {
public:
	/**
	  * Initialize with a given memory block and GPR.
	  * Shares memory & GPR with caller.
	  */
	CPU(char* memory, int memorySize, ISA::Register gpr[]) :
		memory(memory),
		memorySize(memorySize),
		gpr(gpr) {}
	/**
	 * Executes a program.
	 * instructions: a program represented as a series of instructions
	 * instructionBase: absolute base memory address at which instructions are mapped
	 * pc: value of Program Counter at start of execution
	 * Returns whether execution finished successfully (reached halt instruction).
	 */
	bool execute(vector<Instruction> instructions, ISA::Address instructionBase, ISA::Address pc);
	// Returns count of instructions executed so far
	int getInstructionsCount() const;

private:
	// Reads a machine word from memory; offset is in mwords
	ISA::Register readMemory(ISA:Address address);
	// Writes a machine word to memory; offset is in mwords
	void writeMemory(ISA::Register value);
	char* memory;
	int memorySize;
	ISA::Register gpr[ISA::REG_COUNT];
	int instructionsExecuted;
};