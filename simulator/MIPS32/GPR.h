#pragma once

#include "ISA.h"
#include <ostream>

/**
 * MIPS32 general purpose registers.
 */

class GPR {
public:
	typedef struct {
		int value;
		ISA::Tag tag;
	} Reg;
	// Returns whether GPR index is valid
	static bool isValid(int index);
	// Initializes all registers to zero
	#pragma warning(disable:4351)  // gpr array init to zero is the desired behavior
	GPR() : gpr() {}
	// Assumes index is in range
	Reg& operator[](int index);
	// Assumes index is in range
	const Reg& operator[](int index) const;
	// Dump to file, returns whether successful
	bool dump(ostream& out);

	void updateTags(ISA::Tag tag, int value);

private:
	Reg gpr[ISA::REG_COUNT];
	Reg zero;  // Value of zero register
};
