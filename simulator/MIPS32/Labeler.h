#pragma once

#include "ISA.h"
#include <map>
#include <regex>
#include <string>
using namespace std;
using namespace std::tr1;

/**
 * Analyzes assembly source code for jump labels.
 * Maps all labels to absolute memory addresses.
 */
class Labeler {
public:
	typedef map<string, int> Labels;
	// Initialize with absolute base address of code
	Labeler(unsigned int base) : pc(base) {}
	// Parse a single line of assembly code at a given memory address, returns PC after this instruction
	unsigned int parse(string& line);
	const map<string, int>& getLabels() const;

private:
	/**
	 * Matches a labeled instruction, e.g.:
	 * L1: add1 $1 $2 3
	 * Group 1 captures the label.
	 */
	static const regex labeledInstruction;
	/**
	 * Matches an unlabeled instruction, e.g.:
	 * addi $1 $2 3
	 */
	static const regex unlabeledInstruction;

	// Current address
	int pc;
	// Label name to absolute address mapping
	Labels labels;
};