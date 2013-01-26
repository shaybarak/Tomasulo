#include "InstructionQueue.h"
#include "ISA.h"
#include <assert.h>

bool InstructionQueue::tryReadNewInstruction(int now, int pc) {
	if (q.size() >= (unsigned int)depth) {
		// Queue depth exceeded
		return false;
	}
	int instructionIndex;
	// Read from instruction memory
	int notBefore = instructionMemory->read(now, pc * sizeof(int), instructionIndex);
	instructionsRead++;
	// Verify that instruction was correctly read from memory
	assert(ISA::DATA_SEG_SIZE + pc * sizeof(int) == instructionIndex);
	Future<int> futureInstruction(instructionIndex, notBefore);
	q.push(futureInstruction);
	return true;
}

int InstructionQueue::tryGetNextInstruction(int now) {
	if (q.empty()) {
		// Queue is empty
		return -1;
	}
	Future<int> front = q.front();
	if (!front.isReady(now)) {
		// Still busy reading from memory
		return -1;
	}
	return front.get();
}
