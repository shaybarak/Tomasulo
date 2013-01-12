#include "Configuration\Configuration.h"
#include "MIPS32\CPU.h"
#include "MIPS32\InstructionFactory.h"
#include "MIPS32\ISA.h"
#include "MIPS32\Labeler.h"
#include "Clock\Clock.h"
#include "Output\HexDump.h"
#include "Memory\L1Cache.h"
#include "Memory\L2Cache.h"
#include "Memory\PreviousMemoryLevel.h"
#include "Memory\NextMemoryLevel.h"
#include "Memory\MainMemory.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum retvals {
	SUCCESS=0,	// Alles gut!
	BAD_USAGE=1,	// Invalid cmdline invocation
	FIO_ERROR=2,	// File I/O error
	BAD_INPUT=3,	// Bad formatting in one or more input files
	BAD_CONFIG=4,	// Missing configuration value
};

bool openFile(fstream& file, const char* filename, ios_base::openmode openmode) {
	file.open(filename, openmode);
	if (!file) {
		cerr << "Error opening " << filename << endl;
		return false;
	}
	return true;
}

bool openFileRead(fstream& file, const char* filename) {
	return openFile(file, filename, ios_base::in);
};

bool openFileWrite(fstream& file, const char* filename) {
	return openFile(file, filename, ios_base::out);
};

bool readConfig(const Configuration& config, const string& key, int* value) {
	if (!config.get(key, value)) {
		cerr << "Missing configuration " << key << endl;
		return false;
	}
	return true;
}

// Add instructions to memory
void addInstructions(vector<unsigned char>& memory, const vector<Instruction*>& program, int base) {
	int* codePtr = (int*)&memory[base];
	for (unsigned int i = 0; i < program.size(); i++) {
		codePtr[i] = i;
	}
}

// Trim trailing zeroes from memory
void trimMemory(vector<unsigned char>& memory) {
	if (memory.empty()) {
		return;
	}
	// Trim code (not expected in dump)
	memory.resize(ISA::CODE_BASE);
	// Find last non-zero byte
	unsigned char* byte;
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
	fstream cmd_file, config_file, regs_dump, mem_dump, mem_init, 
			time_txt, committed_txt, hitrate, L1i, L1d, L2i, L2d;
	
	if (!openFileRead(cmd_file, argv[1]) || !openFileRead(config_file, argv[2]) ||
		!openFileRead(mem_init, argv[3]) || !openFileWrite(regs_dump, argv[4]) ||
		!openFileWrite(mem_dump, argv[5]) || !openFileWrite(time_txt, argv[6]) ||
		!openFileWrite(committed_txt, argv[7]) || !openFileWrite(hitrate, argv[8]) ||
		!openFileWrite(L1i, argv[9]) || !openFileWrite(L1d, argv[10]) ||
		!openFileWrite(L2i, argv[11]) || !openFileWrite(L2d, argv[12])) {
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
	if (!readConfig(config, "l1_block_size", &l1_block_size) ||
		!readConfig(config, "l1_access_delay", &l1_access_delay) ||
		!readConfig(config, "l1_cache_size", &l1_cache_size) ||
		!readConfig(config, "l2_block_size", &l2_block_size) ||
		!readConfig(config, "l2_access_delay", &l2_access_delay) ||
		!readConfig(config, "l2_cache_size", &l2_cache_size) ||
		!readConfig(config, "mem_access_delay", &mem_access_delay)) {
		return BAD_CONFIG;
	}

	// Initialize memory interfaces
	MasterSlaveInterface cpuL2DataInterface;
	MasterSlaveInterface cpuL2InstInterface;
	
	MasterSlaveInterface l1L2DataInterface;
	MasterSlaveInterface l1L2InstInterface;
	
	MasterSlaveInterface l2RamDataInterface;
	MasterSlaveInterface l2RamInstInterface;

	//Initialize memory units
	//TODO fix!
	//SUPER TODO: fix sizes to half, move "/2" factor in all fields calculations
	//L1Cache	L1InstCache(mem_access_delay, l2_block_size, &l1L2InstInterface);
	//L1Cache	L1DataCache(mem_access_delay, l2_block_size, &l1L2DataInterface);
	//
	//L2Cache	L2InstCache(mem_access_delay, l2_block_size, &l1L2InstInterface);
	//L2Cache	L2DataCache(mem_access_delay, l2_block_size, &l1L2DataInterface);
	
	MainMemory ramInst(mem_access_delay, l2_block_size, &l2RamInstInterface);
	MainMemory ramData(mem_access_delay, l2_block_size, &l2RamDataInterface);
	
	// Read memory initialization
	if (!HexDump::load(*ramData.getBuffer(), mem_init)) {
		cerr << "Error reading memory initialization" << endl;
		return BAD_INPUT;
	}
	mem_init.close();
	// Write instructions to memory
	addInstructions(*ramInst.getBuffer(), program, ISA::CODE_BASE);
	
	// Set up CPU
	/////////////
	// Initialize GPR
	GPR gpr;
	// Initialize CPU
	CPU cpu(ISA::RAM_SIZE, &gpr, &l2RamInstInterface, &l2RamDataInterface);
	cpu.loadProgram(&program, ISA::CODE_BASE, ISA::CODE_BASE>>2);
	Clock sysClock;
	sysClock.addObserver(&cpu);
	//sysClock.addObserver(&l1Cache);
	//TODO uncomment out when l2Cache works
	//sysClock.addObserver(&l2Cache);
	sysClock.addObserver(&ramData);
	sysClock.addObserver(&ramInst);
	
	while (!cpu.isHalted()) {
		sysClock.tick();
	}

	// Write outputs
	////////////////
	// Trim trailing zeroes from memory
	trimMemory(*ramData.getBuffer());
	// Write register dump
	if (!gpr.dump(regs_dump)) {
		cerr << "Error writing registers dump" << endl;
	}
	regs_dump.close();
	// Write memory dumps
	if (/*!HexDump::store(*l1Cache.getInstructionsBuffer(), L1i) ||
		!HexDump::store(*l1Cache.getDataBuffer(), L1d) ||
		!HexDump::store(*l2Cache.getInstructionsBuffer(), L2i) ||
		!HexDump::store(*l2Cache.getDataBuffer(), L2d) || */
		!HexDump::store(*ramData.getBuffer(), mem_dump)) {
		cerr << "Error writing memory dumps" << endl;
	}
	mem_dump.close();
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
