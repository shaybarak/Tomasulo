#pragma once

#include "Instruction.h"

/**
 * A literal J-Type instruction (target is expressed literally).
 */
class JTypeInstruction : public Instruction {
public:
	JTypeInstruction(int pc, ISA::Opcode opcode, int target) :
	  Instruction(pc, opcode), target(target) {}
	int getTarget() const;

private:
	int target;
};
