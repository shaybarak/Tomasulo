#pragma once

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ISA.h"
using namespace std;

/** Instruction abstract base class. */
class Instruction {
public:
	Instruction(const string& stringRepr, int pc, ISA::Opcode opcode)
		: stringRepr(stringRepr), pc(pc), opcode(opcode) {}
	virtual ~Instruction() {}
	const string toString() const { return stringRepr; }
	ISA::Opcode getOpcode() const { return opcode; }
	int getPc() const { return pc; }
	virtual char getRs() const { return 0; }
	int issue;
	int exectue;
	int writeCDB;

private:
	const string stringRepr;
	int pc;
	ISA::Opcode opcode;
};
