#include "Configuration\Configuration.h"
#include "MIPS32\CPU.h"
#include "MIPS32\InstructionFactory.h"
#include "MIPS32\ISA.h"
#include "MIPS32\Labeler.h"
#include "Clock\Clock.h"
#include "Output\HexDump.h"
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
};

bool openFile(ifstream& file, char* filename) {
	cmd_file.open(filename);
	if (!cmd_file) {
		cerr << "Error opening " << argv[1] << endl;
		return false;
	}
	return true;
}

// Add instructions to memory
void addInstructions(vector<char>& memory, int base, int instructionsCount) {
	// Ensure memory is big enough
	memory.resize(ISA::RAM_SIZE);
	int* codePtr = &memory[base];
	for (int i = 0; i < instructionsCount; i++) {
		*codePtr = i;
		codePtr++;
	}
}

// Trim trailing zeroes from memory
void trimMemory(vector<char>& memory) {
	if (memory.empty()) {
		return;
	}
	// Trim code (not expected in dump)
	memory.resize(ISA::CODE_BASE);
	// Find last non-zero byte
	char* byte;
	for (byte = &memory[memory.size() - 1]; byte >= &memory[0]; byte--) {
		if (*byte != 0) break;
	}
	int trimmed = byte - &memory[0] + 1;
	// Align size to multiple of eight, round up
	memory.resize(((trimmed / 8) + (trimmed % 8 == 0 ? 0 : 1)) * 8);
}

// Entry point for the simulator program
// simulator.exe cmd_file.txt config_file.txt mem_init.txt regs_dump.txt \
//     mem_dump.txt time.txt committed.txt hitrate.txt L1i.txt L1d.txt \
//     L2i.txt L2d.txt
int main(int argc, char** argv) {
	// Handle command line arguments
	////////////////////////////////
	if (argc < 13) {
		cout << "Usage: " << argv[0] << " cmd_file.txt config_file.txt mem_init.txt regs_dump.txt"
			 << " mem_dump.txt time.txt committed.txt hitrate.txt L1i.txt L1d.txt"
			 << " L2i.txt L2d.txt" << endl;
		return BAD_USAGE;
	}
	ifstream cmd_file, config_file, mem_init, regs_dump, mem_dump, time_txt,
			 committed, hitrate, L1i, L1d, L2i, L2d;
	if (!openFile(cmd_file, argv[1]) || !openFile(config_file, argv[2]) ||
		!openFile(mem_init, argv[3]) || !openFile(regs_dump, argv[4]) ||
		!openFile(mem_dump, argv[5]) || !openFile(time_txt, argv[6]) ||
		!openFile(committed, argv[7]) || !openFile(hitrate, argv[8]) ||
		!openFile(L1i, argv[9]) || !openFile(L1d, argv[10]) ||
		!openFile(L2i, argv[11]) || !openFile(L2d, argv[12])) {
		return FIO_ERROR;
	}

	// Read inputs
	//////////////
	Labeler labeler(ISA::CODE_BASE);
	// First pass on code: find and process labels
	while (cmd_file) {
		string line;
		getline(cmd_file, line);
		labeler.parse(line);
	}
	InstructionFactory instructionFactory(labeler.getLabels(), ISA::CODE_BASE);
	cmd_file.clear();
	cmd_file.seekg(0);
	// Second pass on code: process instructions
	vector<Instruction*> program;
	while (cmd_file) {
		string line;
		getline(cmd_file, line);
		Instruction* instruction = instructionFactory.parse(line);
		if (instruction != NULL) {
			program.push_back(instruction);
			// BUGBUG instruction pointer never deleted
			// Consider using a scoped container ptr deleter to automate this
		}
	}
	cmd_file.close();
	// Read configuration
	Configuration config;
	config.load(config_file);  // Configuration not used in part 1
	config_file.close();
	// Read memory initialization
	vector<char> memory;
	if (!HexDump::load(memory, mem_init)) {
		cerr << "Error reading memory initialization" << endl;
		return BAD_INPUT;
	}
	fclose(mem_init);
	// Expand memory to maximum size (new bytes are zero-initializes automatically)
	memory.resize(ISA::RAM_SIZE);
	
	// Run program
	//////////////
	// Initialize GPR
	GPR gpr;
	// Execute program
	// WARNING: this dirty trick exposes vector<char>'s internal char[] to CPU.
	// It works because the standard guarantees that vector's internal members are contiguous and packed.
	TimedQueue<int> l1CacheReadQueue();
	TimedQueue<WriteRequest> l1CacheWriteQueue();
	CPU cpu(&l1CacheReadQueue, &l1CacheWriteQueue, 16*1024*1024, &gpr);
	cpu.loadProgram(&program, ISA::CODE_BASE, ISA::CODE_BASE);
	Clock sysClock();
	sysClock.addObserver(&cpu);
	while (!cpu.isHalted()) {
		sysClock.tick();
	}
	// Trim trailing zeroes from memory
	trimMemory(memory);

	// Write outputs
	////////////////
	// Write register dump
	if (!gpr.dump(regs_dump)) {
		cerr << "Error writing registers dump" << endl;
	}
	regs_dump.close();
	// Write memory dump
	if (!HexDump::store(memory, mem_dump)) {
		cerr << "Error writing memory dump" << endl;
	}
	fclose(mem_dump);
	time_txt << sysClock.getTime() << endl;
	if (time_txt.fail()) {
		cerr << "Error writing time" << endl;
	}
	time_txt.close();
	// Write instructions count
	committed_txt << cpu.getInstructionsCommitted() << endl;
	if (committed_txt.fail()) {
		cerr << "Error writing instructions committed count" << endl;
	}
	committed_txt.close();

	return SUCCESS;
}
