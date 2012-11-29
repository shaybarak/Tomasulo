#include "ISA.h"

unsigned int ISA::advancePc(unsigned int pc) {
	return pc + ISA::INSTRUCTION_SIZE / 4;
}