#pragma once

#include "Instruction.h"

/**
 * An unknown instruction.
 */
class UnknownInstruction : public Instruction {
public:
	UnknownInstruction() : Instruction(Instruction::unknown) {}
};