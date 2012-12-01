#include "HexDump.h"
using namespace std;

bool HexDump::load(vector<char>& buffer, FILE* in) {
	char bytes[8];
	while (!feof(in)) {
		buffer.resize(buffer.size() + 8);
		char* base = &buffer[buffer.size() - 8];
		// Line is of format:
		// 00 11 22 33 44 55 66 77
		if (fscanf_s(in, "%02x %02x %02x %02x %02x %02x %02x %02x",
		             base, base+1, base+2, base+3, base+4, base+5, base+6, base+7) != 8) {
			return false;
		}
	}
	return true;
}

bool HexDump::store(vector<char>& buffer, FILE* out) {
	int size = buffer.size();
	// 8 bytes per line
	for (int ch = 0; ch < size - 8; ch+=8) {
		char* base = &buffer[ch];
		if (fprintf(out, "%02x %02x %02x %02x %02x %02x %02x %02x",
			        base, base+1, base+2, base+3, base+4, base+5, base+6, base+7) < 0) {
			return false;
		}
	}
	return true;
}