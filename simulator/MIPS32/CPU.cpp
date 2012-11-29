#include "CPU.h"

bool CPU::execute(vector<Instruction> instructions, ISA::Address instructionBase, ISA::Address pc) {
	return false;
}

int CPU::getInstructionsCount() const {
	return instructionsExecuted;
}