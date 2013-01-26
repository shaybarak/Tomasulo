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
	Instruction(int pc, ISA::Opcode opcode) : pc(pc), opcode(opcode) {}
	virtual ~Instruction() {}
	ISA::Opcode getOpcode() const { return opcode; }
	int getPc() const { return pc; }
	int issue;
	int exectue;
	int writeCDB;

private:
	ISA::Opcode opcode;
	int pc;
};
