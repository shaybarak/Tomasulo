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

	enum MemoryType {
		INST, DATA,
	};

	enum TagType {
		NONE, ADD, MUL, LOAD, STORE,
	};

	typedef struct {
		TagType type;
		int index;
	} Tag;

	// Convert opcode by name to enum value
	static ISA::Opcode toOpcode(const string& opcodeName);
	
	// Count of registers in ISA
	static const int REG_COUNT = 32;
	
	// Size of RAM data segment
	static const int DATA_SEG_SIZE = 15 * 1024 * 1024;

	// Size of RAM code segment
	static const int CODE_SEG_SIZE = 1024 * 1024;
};
