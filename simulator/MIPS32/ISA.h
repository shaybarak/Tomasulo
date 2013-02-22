#pragma once

#include <string>
using namespace std;

/**
 * MIPS32 ISA-level types, methods and definitions.
 */
class ISA {
public:
	// Encode an instruction
	static int encodeInstruction(int pc) { return DATA_SEG_SIZE + pc * sizeof(int); }

	// Decode an instruction
	static int decodeInstruction(int instruction) { return (instruction - DATA_SEG_SIZE) / sizeof(int); }

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
	static const int DATA_SEG_SIZE = 0xF0000;

	// Size of RAM code segment
	static const int CODE_SEG_SIZE = 0x10000;

	// Program base addresses
	static const int FIRST_PROGRAM_BASE = 0xF0000;
	static const int SECOND_PROGRAM_BASE = 0xF8000;
};
