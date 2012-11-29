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
	typedef int Register;
	typedef unsigned int Address;
	static ISA::Opcode toOpcode(const string& opcodeName);
	static unsigned int advancePc(unsigned int pc);
	// Size of instruction in bytes
	static const int INSTRUCTION_SIZE = 4;
	// Count of registers in ISA
	static const int REG_COUNT = 32;
};