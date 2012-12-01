#include "GPR.h"

bool GPR::isValid(int index) {
	return ((0 <= index) && (index < 32));
}

int& GPR::operator[](int index) {
	if (index == 0) {
		return 0;
	}
	return gpr[index];
}

const int& GPR::operator[](int index) const {
	if (index == 0) {
		return 0;
	}
	return gpr[index];
}