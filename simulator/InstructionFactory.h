#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include "Instruction.h"
using namespace std;

/**
 * Factory for parsing assembly code lines and generating an Instruction object out of each.
 */
class InstructionFactory {
public:
	// Parse a single line of assembly code
	static Instruction* parseInstruction(string& line);

private:
	// string->Opcode converter
	static Instruction::Opcode toOpcode(string& opcodeName);
};