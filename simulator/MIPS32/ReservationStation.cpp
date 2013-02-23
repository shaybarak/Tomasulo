#include "ReservationStation.h"
#include "../MIPS32/ISA.h"

ReservationStation::ReservationStation(ISA::TagType tagType, int size, int delay): tagType(tagType), delay(delay) {
	entries.resize(size);
}

ReservationStation::Entry& ReservationStation::operator[](int index) {
	return entries[index];
}

const ReservationStation::Entry& ReservationStation::operator[](int index) const{
	return entries[index];
}

int ReservationStation::getFreeIndex() {
	for (unsigned int i = 0; i < entries.size(); i++) {
		if (!entries[i].busy) {
			return i;
		}
	}
	return -1;
}

void ReservationStation::updateTags(ISA::Tag tag, int value) {
	for (int index = 0; index < entries.size(); index++) {
		if (entries[index].busy == true && entries[index].timeWriteCDB == -1) {
			if (entries[index].qj == tag) {
				entries[index].qj.valid = false;
				entries[index].vj = value;
			}
			if (entries[index].qk == tag) {
				entries[index].qk.valid = false;
				entries[index].vk = value;
			}
		}
	}
}
