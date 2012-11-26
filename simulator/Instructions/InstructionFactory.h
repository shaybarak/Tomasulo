#pragma once

#include <string>
#include <istream>
#include <map>
#include <regex>
#include "Instruction.h"
using namespace std;
using namespace std::tr1;

/**
 * Factory for parsing assembly code lines and generating an Instruction object out of each.
 */
class InstructionFactory {
public:
	// Initialize with a mapping of symbols to immediates
	InstructionFactory(const map<string, int>& symbols) : symbols(symbols) {}
	// Parse a single line of assembly code, on error returns NULL
	Instruction* parseInstruction(string& line);

private:
	// Convert string to Opcode
	static Instruction::Opcode toOpcode(string& opcodeName);
	// Validate register index
	static bool validateRegisterIndex(int index);

	/**
	 * Matches a labeled instruction, e.g.:
	 * L1: add1 $1 $2 3
	 * Group 1 captures the opcode.
	 * Group 2 captures all instruction.
	 */
	static regex labeledInstruction("^\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*)$");
	/**
	 * Matches an unlabeled instruction, e.g.:
	 * addi $1 $2 3
	 * Group 1 captures the opcode.
	 * Group 2 captures all instruction.
	 */
	static regex unlabeledInstruction("^\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*)$");
	
	// Specialized methods for parsing different types of instructions
	Instruction* parseRegisterArithmeticInstruction(Instruction::Opcode opcode, string& arguments);
	Instruction* parseImmediateArithmeticInstruction(Instruction::Opcode opcode, string& arguments);
	Instruction* parseMemoryInstruction(Instruction::Opcode opcode, string& arguments);
	Instruction* parseBranchInstruction(Instruction::Opcode opcode, string& arguments);
	Instruction* parseJumpInstruction(Instruction::Opcode opcode, string& arguments);
	
	// Keep a copy of the symbolic names mapping
	const map<string, int> symbols;
};