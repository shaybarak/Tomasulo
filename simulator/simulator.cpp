#include "Configuration\Configuration.h"
#include "MIPS32\CPU.h"
#include "MIPS32\InstructionFactory.h"
#include "MIPS\ISA.h"
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
	BAD_INPUT,	// Bad formatting in one or more input files
}

// Code is based at address 0
const int CODE_BASE = 0;

// Trim trailing zeroes from memory
int trimMemory(vector<char>& ram) {
	int newSize = ram.size();
	for (vector<char>::reverse_iterator iter = ram.rbegin(); iter != ram.rend(); iter++) {
		if (*iter == 0) {
			newSize--;
		} else {
			// Found last non-zero
			break;
		}
	}
	ram.resize(newSize);
	return newSize;
}

// Entry point for the simulator program
// simulator.exe cmd_file.txt config_file.txt mem_init.txt regs_dump.txt mem_dump.txt time.txt committed.txt
int main(int argc, char** argv) {
	// Handle command line arguments
	////////////////////////////////
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

	// Read inputs
	//////////////
	LabelAnalyzer labeler(CODE_BASE);
	vector<Instruction> program;
	// First pass on code: find and process labels
	while (cmd_file) {
		string line;
		getline(cmd_file, line);
		labeler.parse(line);
	}
	InstructionFactory instructionFactory(labeler.getLabels(), CODE_BASE);
	cmd_file.seekg(beg);
	// Second pass on code: process instructions
	while (cmd_file) {
		string line;
		getline(cmd_file, line);
		Instruction* instruction = instructionFactory.parse(line);
		if (instruction != NULL) {
			program.push_back(*instruction);
			delete instruction;
		}
	}
	cmd_file.close();
	// Read configuration
	Configuration config;
	config.load(config_file);  // Configuration not used in part 1
	config_file.close();
	// Read memory initialization
	vector<char> memory;
	if (!HexDump.load(memory, mem_init)) {
		cerr << "Error reading memory" << endl;
		return BAD_INPUT;
	}
	mem_init.close();
	// Expand memory to maximum size (new bytes are zero-initializes automatically)
	memory.resize(ISA::RAM_SIZE);

	// Run program
	//////////////
	// Initialize GPR
	GPR gpr;
	// Execute program
	// WARNING: this dirty trick exposes vector<char>'s internal char[] to CPU.
	// It works because the standard guarantees that vector's internal members are contiguous and packed.
	CPU cpu(&memory[0], memory.size(), gpr);
	cpu.execute(program, CODE_BASE, CODE_BASE);
	// Trim trailing zeroes from memory
	trimMemory(memory);

	// Write outputs
	////////////////
	// Write register dump
	RegisterDump.store(gpr, ISA::REG_COUNT, regs_dump);
	regs_dump.close();
	// Write memory dump
	HexDump.store(memory, mem_dump);
	mem_dump.close();
	// Write execution time (assumes 1 per instruction committed)
	time_txt << cpu.getInstructionsCommitted() << endl;
	time_txt.close();
	// Write instructions count
	committed_txt << cpu.getInstructionsCommitted() << endl;
	committed_txt.close();

	return SUCCESS;
}