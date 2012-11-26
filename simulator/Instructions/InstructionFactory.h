#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include "Instruction.h"
using namespace std;

/**
 * Factory for parsing assembly code lines and generating an Instruction object out of each.
 */
class InstructionFactory {
public:
	// Initialize with a mapping of symbols to immediates
	InstructionFactory(map<string, int>& symbols) : symbols(symbols) {}
	// Parse a single line of assembly code
	Instruction* parseInstruction(string& line);

private:
	// string->Opcode converter
	static Instruction::Opcode toOpcode(string& opcodeName);
	map<string, int> symbols;
};