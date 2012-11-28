#include "HexDump.h"
#include <iostream>
#include <iomanip>
using namespace std;

bool load(vector<char>& buffer, istream& in) {
	// Configure input stream for hex reading
	ios_base::fmtflags flags = in.flags();
	in >> hex >> setw(2);
	char byte;
	while ((!in.eof()) && (in.good())) {
		in >> byte;
		buffer.push_back(byte);
	}
	// Reset flags
	in.flags(flags);
	return (in.good());
}

bool store(vector<char>& buffer, ostream& out) {
	// Configure output stream for uppercase hex with 0-padded width of 2
	ios_base::fmtflags flags = out.flags();
	out << uppercase << hex << setw(2) << setfill('0');
	int size = buffer.size();
	// 8 bytes per line
	for (int line = 0; line < size / 8; line++) {
		// Handle all bytes in line except the last
		int ch;
		for (ch = 0; ch < 7 && line * 8 + ch < size - 2; ch++) {
			out << buffer[line*8+ch] << " ";
		}
		out << buffer[line*8+ch] << endl;
	}
	// Reset flags
	out.flags(flags);
	return (out.good());
}