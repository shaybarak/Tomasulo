#include "Instruction.h"

/**
 * A generic R-Type instruction.
 */
class RTypeInstruction : public Instruction {
public:
	RTypeInstruction(ISA::Opcode opcode, char rs, char rt, char rd) :
	  Instruction(opcode), rs(rs), rt(rt), rd(rd) {}
	char getRs() const;
	char getRt() const;
	char getRd() const;

private:
	char rs, rt, rd;
};