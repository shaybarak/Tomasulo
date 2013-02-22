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
		int timeIssued;
		//TODO add address for load/store need to add struct in ISA, that includes reg index and offset
	} Entry;

	ReservationStation(ISA::TagType tagType, int size, int delay);

	//Returns the type of the station
	ISA::TagType getTagType() { return tagType; }

	//Returns the delay of functional unit
	int getDelay() { return delay; }

	// Assumes index is in range
	Entry& operator[](int index);
	// Assumes index is in range
	const Entry& operator[](int index) const;

	//returns first free index, -1 if no free index available(rs is full)
	int getFreeIndex();

private:
	//Holds data on instructions
	vector<Entry> entries;

	int delay;

	//The type of the station
	ISA::TagType tagType;

};