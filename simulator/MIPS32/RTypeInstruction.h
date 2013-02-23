#pragma once

#include "Instruction.h"

/**
 * A generic R-Type instruction.
 */
class RTypeInstruction : public Instruction {
public:
	RTypeInstruction(const string& stringRepr, int pc, ISA::Opcode opcode, char rs, char rt, char rd) :
	  Instruction(stringRepr, pc, opcode), rs(rs), rt(rt), rd(rd) {}
	virtual char getRs() const;
	char getRt() const;
	char getRd() const;

private:
	char rs, rt, rd;
};
