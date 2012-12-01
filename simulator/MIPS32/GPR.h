#pragma once

#include "ISA.h"
#include <ostream>

/**
 * MIPS32 general purpose registers.
 */
class GPR {
public:
	// Returns whether GPR index is valid
	static bool isValid(int index);
	// Initializes all registers to zero
	#pragma warning(disable:4351)  // gpr array init to zero is the desired behavior
	GPR() : gpr(), zero(0) {}
	// Assumes index is in range
	int& operator[](int index);
	// Assumes index is in range
	const int& operator[](int index) const;
	// Dump to file, returns whether successful
	bool dump(ostream& out);
private:
	int gpr[ISA::REG_COUNT];
	int zero;
};