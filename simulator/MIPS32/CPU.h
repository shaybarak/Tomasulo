#pragma once

#include "Instruction.h"
#include "GPR.h"
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
	CPU(char* memory, int memorySize, GPR* gpr) :
		memory(memory),
		memorySize(memorySize),
		gpr(gpr),
		instructionsCommitted(0),
		executionTime(0).
		instructions(NULL) {}
	/**
	 * Loads a program.
	 * instructions: a program represented as a series of instructions
	 * instructionBase: absolute base memory address at which instructions are mapped
	 * pc: value of Program Counter at start of execution
	 */
	void loadProgram(vector<Instruction*>* instructions, int instructionBase, int pc);
	/**
	 * Handles clock tick.
	 * time: current clock time.
	 */
	void onTick(int time);
	// Returns count of instructions committed so far
	int getInstructionsCommitted() const;
	// Returns total execution time
	int getExecutionTime() const;
	// Returns whether the CPU has halted due to a halt instruction or an exception
	bool isHalted() const;

private:
	/**
	 * Reads a machine word from memory.
	 * address: offset in machine words
	 * value: output buffer
	 * Returns whether successful.
	 */
	bool readMemory(int address, int* value);
	/**
	 * Writes a machine word to memory.
	 * address: offset in machine words
	 * value: input buffer
	 * Returns whether successful.
	 */
	bool writeMemory(int address, int value);
	char* memory;
	int memorySize;
	GPR* gpr;
	int instructionsCommitted;
	int executionTime;
	vector<Instruction*>* instructions;
	int instructionsBase;
	int pc;
	bool halted;
};
