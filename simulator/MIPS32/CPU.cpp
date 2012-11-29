#include "CPU.h"

bool CPU::execute(vector<Instruction> instructions, ISA::Register instructionBase, ISA::Register startPc) {
	return false;
}

int CPU::getInstructionsCount() const {
	return instructionsExecuted;
}