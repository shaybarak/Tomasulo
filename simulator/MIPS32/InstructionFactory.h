#pragma once

#include <string>
#include <istream>
#include <map>
#include <regex>
#include "ISA.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
using namespace std;
using namespace std::tr1;

/**
 * Factory for parsing assembly code lines and generating an Instruction object out of each.
 */
class InstructionFactory {
public:
	/**
	 * Initializes a factory for generating Instructions out of assembly source code lines.
	 * labels: mapping of label to memory address
	 * startPc: initial value for the Program Counter
	 */
	InstructionFactory(const map<string, ISA::Address>& labels, ISA::Address startPc) : labels(labels), pc(startPc) {}
	// Parses a single line of assembly code, on error returns NULL
	Instruction* parse(string& line);

private:
	// Converts string to Opcode
	static ISA::Opcode toOpcode(const string& opcodeName);
	// Validates register index
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
	RTypeInstruction* parseRegisterArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseImmediateArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseMemoryInstruction(ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseBranchInstruction(ISA::Opcode opcode, const string& arguments) const;
	JTypeInstruction* parseJumpInstruction(ISA::Opcode opcode, const string& arguments) const;
	
	// Keep a copy of the symbolic names mapping
	const map<string, ISA::Address>& labels;
	// Program Counter
	ISA::Address pc;
};