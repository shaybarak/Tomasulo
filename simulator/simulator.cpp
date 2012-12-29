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
	BAD_CONFIG,	// Missing configuration value
};

bool openFile(ifstream& file, const char* filename) {
	cmd_file.open(filename);
	if (!cmd_file) {
		cerr << "Error opening " << argv[1] << endl;
		return false;
	}
	return true;
}

bool readConfig(const Configuration& config, const string& key, int* value) {
	if (!config.get(key, value)) {
		cerr << "Missing configuration " << key << endl;
	}
}

// Add instructions to memory
void addInstructions(vector<char>& memory, const vector<Instruction*>& program, int base) {
	int* codePtr = &memory[base];
	for (int i = 0; i < program.size(); i++) {
		memory.write(base + i, i);
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
	
	// Set up memory levels
	/////////////////////////////
	int l1_block_size, l1_access_delay, l1_cache_size,
		l2_block_size, l2_access_delay, l2_cache_size,
		mem_access_delay;
	if (!readConfig(config, "l1_block_size", l1_block_size) ||
		!readConfig(config, "l1_access_delay", l1_access_delay) ||
		!readConfig(config, "l1_cache_size", l1_cache_size) ||
		!readConfig(config, "l2_block_size", l2_block_size) ||
		!readConfig(config, "l2_access_delay", l2_access_delay) ||
		!readConfig(config, "l2_cache_size", l2_cache_size) ||
		!readConfig(config, "mem_access_delay", mem_access_delay)) {
		return BAD_CONFIG;
	}
	// Initialize memory interfaces
	MemoryInterface cpuL1();
	NextMemoryLevel cpuToL1(cpuL1);
	PreviousMemoryLevel l1ToCpu(cpul1);
	MemoryInterface l1L2();
	NextMemoryLevel l1ToL2(l1L2);
	PreviousMemoryLevel l2ToL1(l1L2);
	MemoryInterface l2MainMemory();
	NextMemoryLevel l2ToMainmemory(l2MainMemory);
	PreviousMemoryLevel mainMemoryToL2(l2MainMemory);
	// Initialize L1
	vector<char> l1Buffer(l1_cache_size);
	L1Cache l1Cache(&l1Buffer[0], l1_block_size, l1_cache_size, l1_access_delay,
		&l1ToCpu, &l1ToL2);
	// Initialize L2
	L2Cache l2Cache(&l2Buffer[0], l2_block_size, l2_cache_size, l2_access_delay,
		&l2ToL1, &l2ToMainmemory);
	// Initialize main memory
	// TODO connect MainMemory to L2, not directly to CPU
	MainMemory ram(mem_access_delay, l2_block_size, &l1ToCpu);

	// Read memory initialization
	if (!HexDump::load(ram.getBuffer(), mem_init)) {
		cerr << "Error reading memory initialization" << endl;
		return BAD_INPUT;
	}
	fclose(mem_init);
	// Write instructions to memory
	addInstructions(ram.getBuffer(), program, ISA::CODE_BASE);
	
	// Set up CPU
	/////////////
	// Initialize GPR
	GPR gpr;
	// Initialize CPU
	CPU cpu(&cpuToL1, ISA::RAM_SIZE, &gpr);
	cpu.loadProgram(&program, ISA::CODE_BASE, ISA::CODE_BASE);
	Clock sysClock();
	sysClock.addObserver(&cpu);
	while (!cpu.isHalted()) {
		sysClock.tick();
	}

	// Write outputs
	////////////////
	// Trim trailing zeroes from memory
	trimMemory(ram.getBuffer());
	// Write register dump
	if (!gpr.dump(regs_dump)) {
		cerr << "Error writing registers dump" << endl;
	}
	regs_dump.close();
	// Write memory dumps
	if (!HexDump::store(l1Cache.getInstructionsBuffer(), L1i) ||
		!HexDump::store(l1Cache.getDataBuffer(), L1d) ||
		!HexDump::store(l2Cache.getInstructionsBuffer(), L2i) ||
		!HexDump::store(l2Cache.getDataBuffer(), L2d) ||
		!HexDump::store(ram.getBuffer(), mem_dump)) {
		cerr << "Error writing memory dumps" << endl;
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
