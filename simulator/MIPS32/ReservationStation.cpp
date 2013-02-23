#include "ReservationStation.h"
#include "../MIPS32/ISA.h"

ReservationStation::ReservationStation(ISA::TagType tagType, int size, int delay): tagType(tagType), delay(delay) {
	entries.resize(size);
	for (int i = 0; i < entries.size(); i++) {
		entries[i].busy = false;
	}
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
	for (unsigned int index = 0; index < entries.size(); index++) {
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

bool ReservationStation::hasPendingInstructions() {
	for (unsigned int index = 0; index < entries.size(); index++) {
		if (entries[index].busy) {
			return true;
		}
	}
	return false;
}

int ReservationStation::findIndexToExecute(int now) {
	int minTimeIssued = now;
	int selectedIndex = -1;
	for (unsigned int index = 0; index < entries.size(); index++ ) {
		if (entries[index].busy && (entries[index].timeWriteCDB < 0) && (!entries[index].qj.valid) && (!entries[index].qk.valid)) {
			if (entries[index].timeIssued < minTimeIssued) {
				minTimeIssued = entries[index].timeIssued;
				selectedIndex = index;
			}
		}
	}
	return selectedIndex;
}

void ReservationStation::execute(int now) {
	int selectedIndex = findIndexToExecute(now);
	if (selectedIndex == -1) {
		return;
	}
	entries[selectedIndex].timeWriteCDB = now + delay - 1;
}
