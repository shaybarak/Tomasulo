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
	// Size of RAM in bytes
	static const int RAM_SIZE = 16 * 1024 * 1024;
	// Base address of code
	static const int CODE_BASE = 15 * 1024 * 1024;
};
