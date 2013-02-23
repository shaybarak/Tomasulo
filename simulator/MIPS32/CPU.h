#pragma once

#include "Instruction.h"
#include "GPR.h"
#include "../Memory/MemorySystem.h"
#include "InstructionQueue.h"
#include "ReservationStation.h"
#include "../Output/Trace.h"
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
	CPU(GPR* gpr, MemorySystem* dataMemory, InstructionQueue* instructionQueue, 
		ReservationStation* rsAddSub, ReservationStation* rsMulDiv, 
		ReservationStation* rsLoad, ReservationStation* rsStore, Trace* trace) :
		gpr(gpr),
		instructionQueue(instructionQueue),
		rsAddSub(rsAddSub),
		rsMulDiv(rsMulDiv),
		rsLoad(rsLoad),
		rsStore(rsStore),
		trace(trace),

		dataMemory(dataMemory),
		now(0),
		instructionsCommitted(0),
		memoryAccessCount(0),
		halted(false) {}
	
	// Executes one instruction
	void runOnce();
	
	// Returns elapsed execution time
	int getTime() { return now; }
	
	// Returns count of instructions committed so far
	int getInstructionsCommitted() const { return instructionsCommitted; }
	
	// Returns count of memory accesses so far
	int getMemoryAccessCount() const { return memoryAccessCount; }
	
	// Returns the Average Memory Access Time
	double getAmat() const { return (double)now / (double)memoryAccessCount; }
	
	// Returns whether the CPU has halted due to a halt instruction or an exception
	bool isHalted() const { return halted; }

	// Flushes all pending operations
	void flush() {  0 / 0; }  // TODO: implement

private:

	// Try to read new instruction from memory to instruction queue
	void fetch(bool issued);

	//Try to issue the instruction just poped from instruction queue
	bool issue(Instruction* instruction);

	//Add an instruction to rs (and updates gpr with new tag)
	void addInstructionToRs(ReservationStation* rs, int index, Instruction* instruction);

	// Pop instruction from queue
	Instruction* decode();

	//Writes value computed by execution units to gpr and reservation stations
	void writeCDB();

	bool writeCDB(ReservationStation* rs);


	//compute return value from reservation station
	int computeValue(ReservationStation* rs, int index);

	// Reads data
	int readData(int address);

	// Writes data
	void writeData(int address, int data);

	/**
	 * Executes an instruction. May stall on memory.
	 * instruction: instruction to execute
	 */
	void execute(Instruction* instruction);
	
	//returns the reservation station matching for the opcode
	ReservationStation* getRs(ISA::Opcode opcode);

	// Size of data memory in bytes (such that memorySize == highest valid address + 1)
	int dataMemorySize;
	// General Purpose Registers
	GPR* gpr;
	// Memory systems
	MemorySystem* dataMemory;

	// Instruction queue
	InstructionQueue* instructionQueue;

	//Reservation stations
	ReservationStation* rsAddSub;
	ReservationStation* rsMulDiv;
	ReservationStation* rsLoad;
	ReservationStation* rsStore;

	// For logging purposes
	Trace* trace;

	// Current cycle
	int now;
	// Count of instructions committed
	int instructionsCommitted;
	// Count of memory reads requested
	int memoryAccessCount;
	// Whether halted (by HALT instruction)
	bool halted;

	int cdbValue;
	ISA::Tag cdbTag;
};
