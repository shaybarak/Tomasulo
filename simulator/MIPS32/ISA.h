#pragma once

#include <string>
using namespace std;

/**
 * MIPS32 ISA-level types, methods and definitions.
 */
class ISA {
public:
	enum Opcode {
		add, sub, mul, div, addi, subi, lw, sw, beq, bne, slt, slti, j, halt, unknown,
	};
	// Convert opcode by name to enum value
	static ISA::Opcode toOpcode(const string& opcodeName);
	// Count of registers in ISA
	static const int REG_COUNT = 32;
};