#include "ReservationStation.h"
#include "../MIPS32/ISA.h"

ReservationStation::ReservationStation(ISA::TagType tagType, int size): tagType(tagType) {
	instructions.resize(size);
}
