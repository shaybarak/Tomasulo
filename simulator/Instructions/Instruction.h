#pragma once

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

/**
 * Instruction abstract base class.
 */
class Instruction {
public:
	enum Opcode {
		add, sub, mul, div, addi, subi, lw, sw, beq, bne, slt, slti, j, halt, unknown,
	};

	Instruction(Opcode opcode) : opcode(opcode) {}
	virtual ~Instruction() = 0;
	Opcode getOpcode() const;

private:
	Opcode opcode;
};