#include "Instruction.h"

/**
 * A generic J-Type instruction.
 */
class JTypeInstruction : public Instruction {
public:
	JTypeInstruction(Instruction::Opcode opcode, int target) :
	  Instruction(opcode), target(target) {}
	int getTarget() const;

private:
	int target;
};