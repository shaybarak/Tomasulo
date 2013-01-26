#pragma once

#include <string>
#include <istream>
#include <map>
#include <regex>
#include "ISA.h"
#include "Labeler.h"
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
	InstructionFactory(const Labeler::Labels& labels, int startPc = 0) : labels(labels), pc(startPc) {}
	// Parses a single line of assembly code, on error returns NULL
	Instruction* parse(string& line);

private:
	// Converts string to Opcode
	static ISA::Opcode toOpcode(const string& opcodeName);

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
	// Matches an empty line or line with whitespaces only
	static const regex emptyLine;
	
	// Specialized methods for parsing different types of instructions
	RTypeInstruction* parseRegisterArithmeticInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseImmediateArithmeticInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseMemoryInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const;
	ITypeInstruction* parseBranchInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const;
	JTypeInstruction* parseJumpInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const;
	
	// Keep a copy of the symbolic names mapping
	const Labeler::Labels& labels;
	// Program Counter
	int pc;
};
