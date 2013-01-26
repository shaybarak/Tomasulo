#include "Configuration\Configuration.h"
#include "MIPS32\CPU.h"
#include "MIPS32\InstructionFactory.h"
#include "MIPS32\ISA.h"
#include "MIPS32\Labeler.h"
#include "Output\HexDump.h"
#include "Memory\L1Cache.h"
#include "Memory\L2Cache.h"
#include "Memory\MainMemory.h"
#include "Memory\MemorySystem.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;

enum retvals {
	SUCCESS=0,		// Alles gut!
	BAD_USAGE=1,	// Invalid cmdline invocation
	FIO_ERROR=2,	// File I/O error
	BAD_INPUT=3,	// Bad formatting in one or more input files
	BAD_CONFIG=4,	// Missing configuration value
};

bool openFileRead(fstream& file, const char* filename) {
	file.open(filename, ios_base::in);
	if (!file) {
		cerr << "Error opening " << filename << endl;
		return false;
	}
	return true;
};

bool openFileWrite(fstream& file, const char* filename) {
	file.open(filename, ios_base::out);
	if (!file) {
		cerr << "Error opening " << filename << endl;
		return false;
	}
	return true;
};

bool readConfig(const Configuration& config, const string& key, int* value) {
	if (!config.get(key, value)) {
		cerr << "Missing configuration " << key << endl;
		return false;
	}
	return true;
}

// Add instructions to memory
void addInstructions(vector<unsigned char>& memory, const vector<Instruction*>& program) {
	int* codePtr = (int*)&memory[0];
	for (unsigned int i = 0; i < program.size(); i++) {
		codePtr[i] = ISA::encodeInstruction(i);
	}
}

// Trim trailing zeroes from memory
void trimMemory(vector<unsigned char>& memory) {
	if (memory.empty()) {
		return;
	}
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
	if (argc < 12) {
		cout << "Usage: " << argv[0] << " cmd_file1.txt cmd_file2.txt config_file.txt mem_init.txt " 
			<< "regs_dump.txt mem_dump.txt time.txt committed.txt  hitrate.txt  trace1.txt trace2.txt" << endl;
		return BAD_USAGE;
	}
	fstream cmd_file1, cmd_file2, config_file, regs_dump, mem_dump, mem_init, 
			time_txt, committed_txt, hitrate, trace1, trace2;
	
	if (!openFileRead(cmd_file1, argv[1]) || !openFileRead(cmd_file2, argv[2]) ||
		!openFileRead(config_file, argv[3]) || !openFileRead(mem_init, argv[4]) || 
		!openFileWrite(regs_dump, argv[5]) || !openFileWrite(mem_dump, argv[6]) || 
		!openFileWrite(time_txt, argv[7]) || !openFileWrite(committed_txt, argv[8]) || 
		!openFileWrite(hitrate, argv[9]) || !openFileWrite(trace1, argv[10]) || 
		!openFileWrite(trace2, argv[11])) {
		
		return FIO_ERROR;
	}

	// Read inputs
	//////////////
	Labeler labeler;
	// First pass on code: find and process labels

	//TODO: change to handle both cmd_files
	while (cmd_file1) {
		string line;
		getline(cmd_file1, line);
		labeler.parse(line);
	}
	InstructionFactory instructionFactory(labeler.getLabels());
	cmd_file1.clear();
	cmd_file1.seekg(0);
	// Second pass on code: process instructions
	vector<Instruction*> program;
	while (cmd_file1) {
		string line;
		getline(cmd_file1, line);
		Instruction* instruction = instructionFactory.parse(line);
		if (instruction != NULL) {
			program.push_back(instruction);
			// BUGBUG instruction pointer never deleted
			// Consider using a scoped container ptr deleter to automate this
		}
	}
	cmd_file1.close();
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

	// Split cache buffer space 50%/50% between instruction and data caches
	L1Cache l1InstCache(ISA::INST, l1_block_size, l1_cache_size / 2, l1_access_delay);
	L1Cache l1DataCache(ISA::DATA, l1_block_size, l1_cache_size / 2, l1_access_delay);
	L2Cache l2InstCache(ISA::INST, l2_block_size, l2_cache_size / 2, l2_access_delay);
	L2Cache l2DataCache(ISA::DATA, l2_block_size, l2_cache_size / 2, l2_access_delay);

	// Initialize RAM
	vector<unsigned char> instructions;
	instructions.resize(ISA::CODE_SEG_SIZE);
	MainMemory ramInst(ISA::INST, (int*)&instructions[0], mem_access_delay);
	vector<unsigned char> data;
	data.resize(ISA::DATA_SEG_SIZE);
	MainMemory ramData(ISA::DATA, (int*)&data[0], mem_access_delay);
	
	// Read memory initialization
	if (!HexDump::load(data, mem_init)) {
		cerr << "Error reading memory initialization" << endl;
		return BAD_INPUT;
	}
	mem_init.close();
	
	// Write instructions to memory
	addInstructions(instructions, program);

	// Build memory systems
	MemorySystem instructionMemory(&l1InstCache, &l2InstCache, &ramInst);
	MemorySystem dataMemory(&l1DataCache, &l2DataCache, &ramData);
	
	// Set up CPU
	/////////////
	// Initialize GPR
	GPR gpr;

	// Initialize instrcution queue
	int iQDepth;
	if (!readConfig(config, "instruction_q_depth", &iQDepth)) {
		return BAD_CONFIG;
	}
	InstructionQueue instructionQueue(iQDepth, &instructionMemory, &program);
	// Initialize CPU
	CPU cpu(&gpr, &dataMemory, &instructionQueue);
	// Run the program until halt is encountered
	while (!cpu.isHalted()) {
		cpu.runOnce();
	}

	// Write outputs
	////////////////
	// Write register dump
	if (!gpr.dump(regs_dump)) {
		cerr << "Error writing registers dump" << endl;
	}
	regs_dump.close();
	// Write memory dumps
	// Trim trailing zeroes from memory
	trimMemory(data);
	if (!HexDump::store(data, mem_dump)) {
		cerr << "Error writing memory dumps" << endl;
	}
	mem_dump.close();
	time_txt << cpu.getTime() << endl;
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
	// Write hit ratios
	int l1Hits = l1InstCache.getHitCount() + l1DataCache.getHitCount();
	int l1Misses = l1InstCache.getMissCount() + l1DataCache.getMissCount();
	int l2Hits = l2InstCache.getHitCount() + l2DataCache.getHitCount();
	int l2Misses = l2InstCache.getMissCount() + l2DataCache.getMissCount();
	hitrate << "L1 " << 100 * l1Hits / (l1Hits + l1Misses) << endl;
	hitrate << "L2 " << 100 * l2Hits / (l2Hits + l2Misses) << endl;
	hitrate << "AMAT " << setprecision(2) << fixed << cpu.getAmat() << endl;

	return SUCCESS;
}
