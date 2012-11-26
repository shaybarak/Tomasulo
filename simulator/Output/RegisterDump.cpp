#include "RegisterDump.h"

bool dump(int* registers, ostream& out) {
	for (int reg = 0; reg < REGISTER_COUNT; reg++) {
		out << "$" << reg << " " << registers[reg] << endl;
	}
	return (out.good());
}