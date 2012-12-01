#pragma once

#include "ISA.h"

/**
 * MIPS32 general purpose registers.
 */
class GPR {
public:
	// Returns whether GPR index is valid
	static bool isValid(int index);
	// Initializes all registers to zero
	GPR() : gpr();
	// Assumes index is in range
	int& operator[](int index);
	// Assumes index is in range
	const int& operator[](int index) const;
private:
	int gpr[ISA::REG_COUNT];
};