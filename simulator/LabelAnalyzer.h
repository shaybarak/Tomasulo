#pragma once

#include <string>
using namespace std;

/**
 * Analyzes assembly source code for jump labels.
 * Maps all labels to absolute memory addresses.
 */
class LabelAnalyzer {
public:
	// Initialize with absolute base address of code
	LabelAnalyzer(unsigned int base) : pc(base) {}
	// Parse a single line of assembly code at a given memory address, returns whether successful
	bool parse(string& line) const;

private:
	// Current address
	unsigned int pc;
};