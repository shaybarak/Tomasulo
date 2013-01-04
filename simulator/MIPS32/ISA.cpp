#include "ISA.h"
#include <algorithm>

bool ISA::isCodeAddress(int address) {
	return address >= CODE_BASE;
}

ISA::Opcode ISA::toOpcode(const string& opcodeName) {
	// Convert opcode to lowercase for comparison
	string opcodeLowered(opcodeName);
	transform(opcodeName.begin(), opcodeName.end(), opcodeLowered.begin(), ::tolower);
	if (opcodeLowered == "add") {
		return ISA::add;
	} else if (opcodeLowered == "sub") {
		return ISA::sub;
	} else if (opcodeLowered == "mul") {
		return ISA::mul;
	} else if (opcodeLowered == "div") {
		return ISA::div;
	} else if (opcodeLowered == "addi") {
		return ISA::addi;
	} else if (opcodeLowered == "subi") {
		return ISA::subi;
	} else if (opcodeLowered == "lw") {
		return ISA::lw;
	} else if (opcodeLowered == "sw") {
		return ISA::sw;
	} else if (opcodeLowered == "beq") {
		return ISA::beq;
	} else if (opcodeLowered == "bne") {
		return ISA::bne;
	} else if (opcodeLowered == "slt") {
		return ISA::slt;
	} else if (opcodeLowered == "slti") {
		return ISA::slti;
	} else if (opcodeLowered == "j") {
		return ISA::j;
	} else if (opcodeLowered == "halt") {
		return ISA::halt;
	} else {
		return ISA::unknown;
	}
}