#include "InstructionFactory.h"

Instruction& parseInstruction(string& line) {
	string opcode;
	istringstream lineStream(line);
	// Parse opcode
	lineStream >> opcode;

}

InstructionFactory::Opcode toOpcode(string& opcode) {
	if (opcode == "add") {
		return InstructionFactory::add;
	} else if (opcode == "sub") {
		return InstructionFactory::sub;
	} else if (opcode == "mul") {
		return InstructionFactory::mul;
	} else if (opcode == "div") {
		return InstructionFactory::div;
	} else if (opcode == "addi") {
		return InstructionFactory::addi;
	} else if (opcode == "subi") {
		return InstructionFactory::subi;
	} else if (opcode == "lw") {
		return InstructionFactory::lw;
	} else if (opcode == "sw") {
		return InstructionFactory::sw;
	} else if (opcode == "beq") {
		return InstructionFactory::beq;
	} else if (opcode == "bne") {
		return InstructionFactory::bne;
	} else if (opcode == "slt") {
		return InstructionFactory::slt;
	} else if (opcode == "slti") {
		return InstructionFactory::slti;
	} else if (opcode == "j") {
		return InstructionFactory::j;
	} else if (opcode == "halt") {
		return InstructionFactory::halt;
	} else {
		return InstructionFactory::unknown;
	}
}