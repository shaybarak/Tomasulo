#pragma once

/**
 * MIPS32 ISA-level types, methods and definitions.
 */
class ISA {
public:
	enum Opcode {
		add, sub, mul, div, addi, subi, lw, sw, beq, bne, slt, slti, j, halt, unknown,
	};
	static unsigned int advancePc(unsigned int pc);
	// Size of instruction in bits
	static const int INSTRUCTION_SIZE = 32;
	// Count of registers in ISA
	static const int REG_COUNT = 32;
};