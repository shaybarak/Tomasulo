#pragma once

#include "Instruction.h"
#include "GPR.h"
#include "../Clock/Clocked.h"
#include "../Memory/NextMemoryLevel.h"
#include <vector>
using namespace std;

/**
 * MIPS32 CPU.
 * No pipelining.
 * Single memory level (no caches).
 */
class CPU : public Clocked {
public:
	/**
	  * Initialize with a given memory block and GPR.
	  * Shares memory & GPR with caller.
	  */
	CPU(NextMemoryLevel<int, int>* nextMemoryLevel, int memorySize, GPR* gpr) :
		nextMemoryLevel(nextMemoryLevel),
		memorySize(memorySize),
		gpr(gpr),
		now(0),
		instructionsCommitted(0),
		instructions(NULL),		
		instructionReadStall(false),
		dataReadStall(false),
		halted(false) {}
	/**
	 * Loads a program.
	 * instructions: a program represented as a series of instructions
	 * instructionsBase: absolute base memory address at which instructions are mapped
	 * pc: value of Program Counter at start of execution
	 */
	void loadProgram(vector<Instruction*>* instructions, int instructionsBase, int pc);
	// Clock tick handler
	virtual void onTick(int now);
	// Returns count of instructions committed so far
	int getInstructionsCommitted() const;
	// Returns total execution time
	int getExecutionTime() const;
	// Returns whether the CPU has halted due to a halt instruction or an exception
	bool isHalted() const;

private:
	/**
	 * Executes an instruction. May stall on memory.
	 * instructionIndex: position of instruction in program
	 */
	void execute(int instructionIndex);
	/**
	 * Executes an instruction with a given memory word.
	 * instructionIndex: position of instruction in program
	 * data: data from memory for instruction
	 */
	void execute(int instructionIndex, int data);
	/** Returns whether the address is a valid memory offset. */
	bool isValidMemoryAddress(int address);
	/** Returns whether the address is a valid memory offset for instructions. */
	bool isValidInstructionAddress(int address);
	/** Converts PC value to memory offset. */
	int pcToMemoryOffset(int pc);
	/** Converts memory offset to index of instruction in program. */
	int memoryOffsetToInstructionIndex(int pc);
	// Next memory level (e.g. L1 cache)
	NextMemoryLevel<int, int>* nextMemoryLevel;
	// Size of memory (such that memorySize == highest valid address + 1)
	int memorySize;
	// General {urpose Registers
	GPR* gpr;
	// Current cycle
	int now;
	// Count of instructions committed
	int instructionsCommitted;
	// Program to execute
	vector<Instruction*>* instructions;
	// Base address of instructions in memory
	int instructionsBase;
	// Program Counter
	int pc;
	// Whether stalled on reading instruction from memory
	bool instructionReadStall;
	// Whether stalled on reading data from memory
	bool dataReadStall;
	// TODO are there write stalls?
	// Whether halted (by HALT instruction)
	bool halted;
};
