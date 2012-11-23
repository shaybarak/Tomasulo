#include <string>
#include <iostream>
#include <sstream>
#include "Instruction.h"
using namespace std;

class InstructionFactory {
public:
	// Parse a single line of assembly code
	Instruction& parseInstruction(string& line);

	enum Opcode {
		add, sub, mul, div, addi, subi, lw, sw, beq, bne, slt, slti, j, halt, unknown,
	};

private:
	// string->Opcode converter
	Opcode toOpcode(string& opcode);
};