#include "InstructionQueue.h"
#include "ISA.h"
#include "JTypeInstruction.h"
#include <assert.h>

bool InstructionQueue::isDone() {
	return halted2;
}

bool InstructionQueue::tryFetch(int now) {
	if (halted2) {
		// Halt!
		return false;
	}
	if (branched) {
		// Throttled due to branch (waiting for beq/bne to finish execution to decide)
		return false;
	}
	if (q.size() >= (unsigned int)depth) {
		// Queue depth exceeded
		return false;
	}
	int instructionIndex;
	// Read from instruction memory
	int notBefore = instructionMemory->read(now, pcToAddress(pc), instructionIndex);
	// Translate instruction encoding
	instructionIndex = ISA::decodeInstruction(instructionIndex);
	// Verify that instruction was correctly read from memory
	assert(pc == instructionIndex);
	Instruction* instruction = getInstruction(instructionIndex);
	Future<Instruction*> futureInstruction(instruction, notBefore);
	q.push(futureInstruction);
	// Update PC
	pc++;
	switch (instruction->getOpcode()) {
	case ISA::j:
		// Jump to next instruction
		pc = dynamic_cast<JTypeInstruction*>(instruction)->getTarget();
	case ISA::beq:
	case ISA::bne:
		// Need to evaluate condition; no prediction so just wait
		branched = true;
		break;
	case ISA::halt:
		// Halt!
		halt();
	default:
		break;
	}
	return true;
}

Instruction* InstructionQueue::tryGetNextInstruction(int now) {
	if (q.empty()) {
		// Queue is empty
		return NULL;
	}
	Future<Instruction*> front = q.front();
	if (!front.isReady(now)) {
		// Still busy reading from memory
		return NULL;
	}
	return front.get();
}

void InstructionQueue::popNextInstruction() {
	q.pop();
}

int InstructionQueue::pcToAddress(int pc) {
	int address = pc * sizeof(int);
	if (!executing1) {
		// Add offset to base of second program
		address += ISA::SECOND_PROGRAM_BASE - ISA::FIRST_PROGRAM_BASE;
	}
	// Addressing is based at start of code segment
	return address;
}

Instruction* InstructionQueue::getInstruction(int instructionIndex) {
	if (executing1) {
		program1->at(instructionIndex);
	} else {
		program2->at(instructionIndex);
	}
}

void InstructionQueue::halt() {
	// If currently executing program 1
	if (executing1 && !program2->empty()) {
		// Switch to program 2
		executing1 = false;
	} else {
		// Finished all programs, halt
		halted2 = true;
	}
}
