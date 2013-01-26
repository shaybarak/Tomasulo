#pragma once
#include "ISA.h"
#include "Instruction.h"
#include <vector>
using namespace std;

class ReservationStation {
public:
	typedef struct {
		Instruction* pInstruction;
		bool busy;
		int vj;
		int vk;
		ISA::Tag qj;
		ISA::Tag qk;
	} InstructionStatus;

	vector<InstructionStatus> instructions;

	ISA::TagType tagType;

};