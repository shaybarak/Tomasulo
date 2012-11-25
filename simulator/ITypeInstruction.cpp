#include "ITypeInstruction.h"

char ITypeInstruction::getRs() const {
	return rs;
}

char ITypeInstruction::getRt() const {
	return rt;
}

short ITypeInstruction::getImmediate() const {
	return immediate;
}