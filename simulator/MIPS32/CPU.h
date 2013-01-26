#pragma once

#include "Instruction.h"
#include "GPR.h"
#include "../Memory/MemorySystem.h"
#include "InstructionQueue.h"
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
	CPU(GPR* gpr, MemorySystem* dataMemory, InstructionQueue* instructionQueue) :
		gpr(gpr),
		instructionQueue(instructionQueue),
		dataMemory(dataMemory),
		now(0),
		instructionsCommitted(0),
		memoryAccessCount(0),
		instructions(NULL),		
		halted(false) {}
	/**
	 * Loads a program.
	 * instructions: a program represented as a series of instructions
	 * pc: value of Program Counter at start of execution, default of zero
	 */
	void loadProgram(vector<Instruction*>* instructions, int pc = 0);
	// Executes one instruction
	virtual void runOnce();
	// Returns elapsed execution time
	int getTime() { return now; }
	// Returns count of instructions committed so far
	int getInstructionsCommitted() const { return instructionsCommitted; }
	// Returns the Average Memory Access Time
	double getAmat() const { return (double)now / (double)memoryAccessCount; }
	// Returns whether the CPU has halted due to a halt instruction or an exception
	bool isHalted() const { return halted; }

private:
	// Reads an instruction from the PC
	void readInstruction();

	// Reads data
	int readData(int address);

	// Writes data
	void writeData(int address, int data);

	/**
	 * Executes an instruction. May stall on memory.
	 * instruction: instruction to execute
	 */
	void execute(Instruction* instruction);
	
	// Size of data memory in bytes (such that memorySize == highest valid address + 1)
	int dataMemorySize;
	// General Purpose Registers
	GPR* gpr;
	// Memory systems
	MemorySystem* dataMemory;

	// Instruction queue
	InstructionQueue* instructionQueue;

	// Current cycle
	int now;
	// Count of instructions committed
	int instructionsCommitted;
	// Count of memory reads requested
	int memoryAccessCount;
	// Whether halted (by HALT instruction)
	bool halted;
	// Program to execute
	vector<Instruction*>* instructions;
	// Program Counter
	int pc;
};
