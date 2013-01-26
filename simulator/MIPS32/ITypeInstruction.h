#pragma once

#include "Instruction.h"

/**
 * A generic I-Type instruction.
 */
class ITypeInstruction : public Instruction {
public:
	ITypeInstruction(int pc, ISA::Opcode opcode, char rs, char rt, short immediate) :
	  Instruction(pc, opcode), rs(rs), rt(rt), immediate(immediate) {}
	char getRs() const;
	char getRt() const;
	short getImmediate() const;

private:
	char rs, rt;
	short immediate;
};
