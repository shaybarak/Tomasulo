#include "GPR.h"

bool GPR::isValid(int index) {
	return ((0 <= index) && (index < 32));
}

int& GPR::operator[](int index) {
	if (index == 0) {
		zero = 0;
		return zero;
	}
	return gpr[index];
}

const int& GPR::operator[](int index) const {
	if (index == 0) {
		*const_cast<int*>(&zero) = 0;
		return zero;
	}
	return gpr[index];
}

bool GPR::dump(ostream& out) {
	for (int i = 0; i < ISA::REG_COUNT; i++) {
		out << "$" << i << " " << gpr[i] << endl;
	}
	return (out.good());
}