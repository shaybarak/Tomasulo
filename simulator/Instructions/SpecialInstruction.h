#include "Instruction.h"

/**
 * Special instruction with no arguments.
 */
class SpecialInstruction : public Instruction {
public:
	SpecialInstruction(Instruction::Opcode opcode) : Instruction(opcode) {}
};