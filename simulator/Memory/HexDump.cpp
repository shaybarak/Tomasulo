#include "HexDump.h"

bool dumpToFile(char* buffer, int length, ostream& out) {
	// 8 bytes per line
	for (int line = 0; line < length / 8; line++) {
		// Handle all bytes in line except the last
		for (int ch = 0; ch < 7 && line * 8 + ch < length - 2; ch++) {
			out << uppercase(hex(buffer[line*8+ch])) << " ";
		}
		out << uppercase(hex(buffer[line*8+ch], 2)) << endl;
	}
	return (out.good());
}