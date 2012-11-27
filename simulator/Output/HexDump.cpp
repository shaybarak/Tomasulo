#include "HexDump.h"
using namespace std;

bool load(vector<char>& buffer, istream& in) {
	while ((!in.eof()) && (in.good())) {
		char byte;
		in >> hex(byte, 2);
		buffer.push_back(byte);
	}
	return (in.good());
}

bool store(vector<char>& buffer, ostream& out) {
	int size = buffer.size();
	// 8 bytes per line
	for (int line = 0; line < size / 8; line++) {
		// Handle all bytes in line except the last
		int ch;
		for (ch = 0; ch < 7 && line * 8 + ch < size - 2; ch++) {
			out << uppercase(hex(buffer[line*8+ch])) << " ";
		}
		out << uppercase(hex(buffer[line*8+ch], 2)) << endl;
	}
	return (out.good());
}