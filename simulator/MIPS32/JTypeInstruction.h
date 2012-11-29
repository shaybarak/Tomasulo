#include "Instruction.h"

/**
 * A literal J-Type instruction (target is expressed literally).
 */
class JTypeInstruction : public Instruction {
public:
	JTypeInstruction(ISA::Opcode opcode, int target) :
	  Instruction(opcode), target(target) {}
	int getTarget() const;

private:
	int target;
};