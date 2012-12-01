#include "HexDump.h"
using namespace std;

bool HexDump::load(vector<char>& buffer, FILE* in) {
	while (!feof(in)) {
		int input[8];
		// Line is of format:
		// 00 11 22 33 44 55 66 77
		int matches = fscanf_s(in, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
		                       input, input+1, input+2, input+3, input+4,
			                   input+5, input+6, input+7);
		if (matches != 8) {
			return false;
		}
		for (int i = 0; i < 8; i++) {
			buffer.push_back(input[i]);
		}
	}
	return true;
}

bool HexDump::store(vector<char>& buffer, FILE* out) {
	int size = buffer.size();
	// 8 bytes per line
	for (int ch = 0; ch <= size - 8; ch+=8) {
		char* base = &buffer[ch];
		if (fprintf(out, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
			        base[0], base[1], base[2], base[3],
					base[4], base[5], base[6], base[7]) < 0) {
			return false;
		}
	}
	return true;
}