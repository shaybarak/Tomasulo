#include "RegisterDump.h"

using namespace std;

bool RegisterDump::store(ISA::Register* registers, int count, ostream& out) {
	for (int reg = 0; reg < count; reg++) {
		out << "$" << reg << " " << registers[reg] << endl;
	}
	return (out.good());
}