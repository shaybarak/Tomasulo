#include "RegisterDump.h"

using namespace std;

// Count of registers in ISA
static const int REGISTER_COUNT = 32;

bool dump(int* registers, ostream& out) {
	for (int reg = 0; reg < REGISTER_COUNT; reg++) {
		out << "$" << reg << " " << registers[reg] << endl;
	}
	return (out.good());
}