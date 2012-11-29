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
	// Initialize with a mapping of labels to immediates
	InstructionFactory(const map<string, int>& labels) : labels(labels) {}
	// Parse a single line of assembly code at a given memory address, on error returns NULL
	Instruction* parse(string& line, unsigned int address) const;

private:
	// Convert string to Opcode
	static ISA::Opcode toOpcode(const string& opcodeName);
	// Validate register index
	static bool validateRegisterIndex(int index);

	/**
	 * Matches a labeled instruction, e.g.:
	 * L1: add1 $1 $2 3
	 * Group 1 captures the opcode.
	 * Group 2 captures all instruction.
	 */
	static const regex labeledInstruction;
	/**
	 * Matches an unlabeled instruction, e.g.:
	 * addi $1 $2 3
	 * Group 1 captures the opcode.
	 * Group 2 captures all instruction.
	 */
	static const regex unlabeledInstruction;
	
	// Specialized methods for parsing different types of instructions
	Instruction* parseRegisterArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const;
	Instruction* parseImmediateArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const;
	Instruction* parseMemoryInstruction(ISA::Opcode opcode, const string& arguments) const;
	Instruction* parseBranchInstruction(ISA::Opcode opcode, const string& arguments) const;
	Instruction* parseJumpInstruction(ISA::Opcode opcode, const string& arguments) const;
	
	// Keep a copy of the symbolic names mapping
	const map<string, int> labels;
};