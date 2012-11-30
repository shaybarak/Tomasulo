#include "Configuration\Configuration.h"
#include "MIPS32\CPU.h"
#include "MIPS32\InstructionFactory.h"
#include "MIPS32\LabelAnalyzer.h"
#include "Output\HexDump.h"
#include "Output\RegisterDump.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum retvals {
	SUCCESS,	// Alles gut!
	BAD_USAGE,	// Invalid cmdline invocation
	FIO_ERROR,	// File I/O error
}

// Entry point for the simulator program
// simulator.exe cmd_file.txt config_file.txt mem_init.txt regs_dump.txt mem_dump.txt time.txt committed.txt
int main(int argc, char** argv) {
	// Handle command line arguments
	if (argc < 8) {
		cout << "Usage: " << argv[0] << " cmd_file config_file mem_init regs_dump mem_dump time_txt committed_txt" << endl;
		return BAD_USAGE;
	}
	ifstream cmd_file;
	cmd_file.open(argv[1]);
	if (!cmd_file) {
		cerr << "Error reading from " << argv[1] << endl;
		return FIO_ERROR;
	}
	ifstream config_file;
	config_file.open(argv[2]);
	if (!config_file) {
		cerr << "Error reading from " << argv[2] << endl;
		return FIO_ERROR;
	}
	ifstream mem_init;
	mem_init.open(argv[3]);
	if (!mem_init) {
		cerr << "Error reading from " << argv[3] << endl;
		return FIO_ERROR;
	}
	ifstream regs_dump;
	regs_dump.open(argv[4]);
	if (!regs_dump) {
		cerr << "Error reading from " << argv[4] << endl;
		return FIO_ERROR;
	}
	ofstream mem_dump;
	mem_dump.open(argv[5]);
	if (!mem_dump) {
		cerr << "Error reading from " << argv[5] << endl;
		return FIO_ERROR;
	}
	ofstream time_txt;
	time_txt.open(argv[6]);
	if (!time_txt) {
		cerr << "Error reading from " << argv[6] << endl;
		return FIO_ERROR;
	}
	ifstream committed_txt;
	committed_txt.open(argv[7]);
	if (!committed_txt) {
		cerr << "Error reading from " << argv[7] << endl;
		return FIO_ERROR;
	}
	return SUCCESS;
}