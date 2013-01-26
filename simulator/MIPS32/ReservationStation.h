#pragma once
#include "ISA.h"
#include "Instruction.h"
#include <vector>
using namespace std;

class ReservationStation {
public:

	// Holds data on instruction status
	typedef struct {
		Instruction* pInstruction;
		bool busy;
		int vj;
		int vk;
		ISA::Tag qj;
		ISA::Tag qk;
		//TODO add address for load/store need to add struct in ISA, that includes reg index and offset
	} InstructionStatus;

	ReservationStation(ISA::TagType tagType, int size);

	//Returns the type of the station
	ISA::TagType getTagType() {return tagType;}

private:
	//Holds data on instructions
	vector<InstructionStatus> instructions;

	//The type of the station
	ISA::TagType tagType;

};