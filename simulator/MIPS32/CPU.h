#pragma once

#include "Instruction.h"
#include "GPR.h"
#include "../Clock/Clocked.h"
#include "../Memory/MasterSlaveInterface.h"
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
	CPU(int memorySize, GPR* gpr, MasterSlaveInterface* pInstInterface, MasterSlaveInterface* pDataInterface) :
		memorySize(memorySize),
		gpr(gpr),
		pL1DataSlave(pInstInterface),
		pL1InstSlave(pDataInterface),
		state(HALT),
		now(0),
		instructionsCommitted(0),
		memoryAccessCount(0),
		instructions(NULL),		
		instructionReadStall(false),
		dataReadStall(false),
		halted(false), 
		timeStalledOnMemory(0),
		memoryReadsCount(0) {}
	/**
	 * Loads a program.
	 * instructions: a program represented as a series of instructions
	 * pc: value of Program Counter at start of execution, default of zero
	 */
	void loadProgram(vector<Instruction*>* instructions, int pc = 0);
	// Clock tick handlers
	virtual void onTickUp(int now);
	virtual void onTickDown(int now);
	// Returns count of instructions committed so far
	int getInstructionsCommitted() const { return instructionsCommitted; }
	// Returns the Average Memory Access Time
	double getAmat() const { return (double)now / memoryAccessCount; }
	// Returns whether the CPU has halted due to a halt instruction or an exception
	bool isHalted() const { return halted; }

private:
	enum CpuState {
		READY,		//initial state, and after execution
		INST_STALL,	//stalled on instruction read from L1
		LW,			//after reading lw instrcution
		LW_STALL,	//stalled on data read from L1
		SW,			//after reading sw instruction 
		SW_STALL,	//stalled on data write to L1
		HALT,		//read halt opcode
	};

	/**
	 * Executes an instruction. May stall on memory.
	 * instructionIndex: position of instruction in program
	 */
	void execute(int instructionIndex);
	/** continue execution of lw when memory call returns **/
	void continueExecuteLw();
	/** continue execution of sw when memory call returns **/
	void continueExecuteSw();
	/** Returns whether the address is a valid memory offset. */
	bool isValidMemoryAddress(int address);
	/** Returns whether the address is a valid memory offset for instructions. */
	bool isValidInstructionAddress(int address);
	/** Converts PC value to memory offset. */
	int pcToMemoryOffset(int pc);
	/** set memory interface signals for reading instrcution **/
	void requestReadInst();
	/** set memory interface signals for reading data **/
	void requestReadData(int address);
	/** set memory interface signals for writing instrcution **/
	void requestWrite(int address, int data);
	// Size of memory (such that memorySize == highest valid address + 1)
	int memorySize;
	// General {urpose Registers
	GPR* gpr;
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
	// Whether stalled on reading instruction from memory
	bool instructionReadStall;
	// Whether stalled on reading data from memory
	bool dataReadStall;
	//counter for stalled memory time
	int timeStalledOnMemory;
	//counter for memory reads count
	int memoryReadsCount;
	//Next address to read
	int nextLwAddress;
	//Next Rt to write data to
	int nextLwRt;
	//Next address to write
	int nextSwAddress;
	//Next data to write
	int nextSwData;
	//Interfaces for communicating with L1 caches
	MasterSlaveInterface* pL1DataSlave;
	MasterSlaveInterface* pL1InstSlave;
	//CPU internal state
	CpuState state;
};
