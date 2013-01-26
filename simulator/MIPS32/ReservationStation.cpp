#include "ReservationStation.h"
#include "../MIPS32/ISA.h"

ReservationStation::ReservationStation(ISA::TagType tagType, int size, int delay): tagType(tagType), delay(delay) {
	instructions.resize(size);
}
