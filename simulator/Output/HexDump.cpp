#include "HexDump.h"
#include <iomanip>
using namespace std;

// Shortcut for outputting a byte as two-digit 0-padded hexadecimal
#define HEX(x) hex << setw(2) << setfill('0') << (int)x

bool HexDump::load(vector<unsigned char>& buffer, fstream& in) {
	unsigned char* ptr = &buffer[0];
	while (!in.eof()) {
		// Line is of format:
		// 00 11 22 33 44 55 66 77
		int temp;
		for (int i=0; i<8; i++) {
			in >> hex >> temp;
			if (in.bad()) {
				return false;			
			}
			*ptr++ = temp;
		}
	}
	return true;
}

bool HexDump::store(const vector<unsigned char>& buffer, fstream& out) {
	if (buffer.size() == 0) {
		return true;
	}
	// 8 bytes per line
	for (unsigned char* base = (unsigned char*)&buffer[0]; base <= &buffer[0] + buffer.size() - 8; base += 8) {
		out << HEX(base[0]) << " " << HEX(base[1]) << " " << HEX(base[2]) << " " << HEX(base[3]) << " "
		    << HEX(base[4]) << " " << HEX(base[5]) << " " << HEX(base[6]) << " " << HEX(base[7]) << endl;
		if (out.fail()) {
			return false;
		}
	}
	return true;
}
