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
	for (int i = 0; i < entries.size(); i++) {
		if (!entries[i].busy) {
			return i;
		}
		return -1;
	}
}
