#pragma once

#include "Instruction.h"

/**
 * A generic R-Type instruction.
 */
class RTypeInstruction : public Instruction {
public:
	RTypeInstruction(int pc, ISA::Opcode opcode, char rs, char rt, char rd) :
	  Instruction(pc, opcode), rs(rs), rt(rt), rd(rd) {}
	char getRs() const;
	char getRt() const;
	char getRd() const;

private:
	char rs, rt, rd;
};
