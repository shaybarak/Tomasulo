#include "HexDump.h"
using namespace std;

bool HexDump::load(vector<unsigned char>& buffer, FILE* in) {
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

bool HexDump::store(const vector<unsigned char>& buffer, FILE* out) {
	// 8 bytes per line
	for (unsigned char* base = (unsigned char*)&buffer[0]; base <= &buffer[0] + buffer.size() - 8; base+=8) {
		if (fprintf(out, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
			        base[0], base[1], base[2], base[3],
					base[4], base[5], base[6], base[7]) < 0) {
			return false;
		}
	}
	return true;
}

bool HexDump::load(vector<unsigned char>& buffer, fstream& in) {
	while (in.eof()) {
		// Line is of format:
		// 00 11 22 33 44 55 66 77
		int temp;
		for (int i=0; i<8; i++) {
			in >> hex >> temp;
			buffer.push_back(temp);
			if (in.bad()) {
				return false;			
			}
		}
	}
	return true;
}


bool HexDump::store(const vector<unsigned char>& buffer, fstream& out) {
	for (unsigned char* base = (unsigned char*)&buffer[0]; base <= &buffer[0] + buffer.size() - 8; base+=8) {
		out << hex << (int)base[0] << (int)base[1] << (int)base[2] << (int)base[3] <<
						(int)base[4] << (int)base[5] << (int)base[6] << (int)base[7] << endl;
		if (out.fail()) {
			return false;
		}
		out << dec;
	}
	return true;
}