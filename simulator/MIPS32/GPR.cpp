#include "GPR.h"

bool GPR::isValid(int index) {
	return ((0 <= index) && (index < 32));
}

GPR::Reg& GPR::operator[](int index) {
	return gpr[index];
}

const GPR::Reg& GPR::operator[](int index) const {
	return gpr[index];
}

bool GPR::dump(ostream& out) {
	for (int i = 0; i < ISA::REG_COUNT; i++) {
		out << "$" << i << " " << gpr[i].value << endl;
	}
	return (out.good());
}
