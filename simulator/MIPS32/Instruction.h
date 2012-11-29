#pragma once

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ISA.h"
using namespace std;

/**
 * Instruction abstract base class.
 */
class Instruction {
public:
	Instruction(ISA::Opcode opcode) : opcode(opcode) {}
	virtual ~Instruction() {}
	ISA::Opcode getOpcode() const;

private:
	ISA::Opcode opcode;
};