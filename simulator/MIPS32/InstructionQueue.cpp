#include "InstructionQueue.h"
#include "ISA.h"
#include "JTypeInstruction.h"
#include <assert.h>

bool InstructionQueue::tryReadNewInstruction(int now) {
	if (halted) {
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
	int notBefore = instructionMemory->read(now, pc * sizeof(int), instructionIndex);
	// Translate instruction encoding
	instructionIndex = ISA::decodeInstruction(instructionIndex);
	// Verify that instruction was correctly read from memory
	assert(pc == instructionIndex);
	Instruction* instruction = instructions->at(instructionIndex);
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
		halted = true;
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
	q.pop();
	return front.get();
}
