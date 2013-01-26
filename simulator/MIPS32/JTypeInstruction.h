#pragma once

#include "Instruction.h"

/**
 * A literal J-Type instruction (target is expressed literally).
 */
class JTypeInstruction : public Instruction {
public:
	JTypeInstruction(const string& stringRepr, int pc, ISA::Opcode opcode, int target) :
	  Instruction(stringRepr, pc, opcode), target(target) {}
	int getTarget() const;

private:
	int target;
};
