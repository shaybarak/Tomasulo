#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include "SpecialInstruction.h"
#include "UnknownInstruction.h"

Instruction* parseInstruction(string& line) {
	string opcodeName;
	istringstream lineStream(line);
	// Parse opcode
	lineStream >> opcodeName;
	if (lineStream.rdstate() == ios::failbit) {
		cerr << "Parse error on opcode name in line " << line << endl;
		return NULL;
	}
	Instruction::Opcode opcode = toOpcode(opcodeName);
	char rest[21];
	lineStream.getline(rest, sizeof(rest) / sizeof(char));
	if (lineStream.rdstate() == ios::failbit) {
		cerr << "Rest of line too long in line " << line << endl;
		return NULL;
	}
	switch (opcode) {
	add:
	sub:
	mul:
	div:
	slt:
		// Handle R-Type instruction
		char rs, rt, rd;
		return new RTypeInstruction(opcode, rs, rt, rd);
	addi:
	subi:
	lw:
	sw:
	beq:
	bne:
	slti:
		// Handle I-Type instruction
		char rs, rt;
		short immediate;
		linestream >> 
		return new ITypeInstruction(opcode, rs, rt, immediate);
	j:
		// Handle J-Type instruction
		int target;
		return new JTypeInstruction(opcode, target);
	halt:
		// Handle special instruction
		return new SpecialInstruction(Instruction::halt);
	default:
		// Unidentified instruction (bug? error?)
		return new UnknownInstruction();
	}
}

static Instruction::Opcode toOpcode(string& opcode) {
	if (opcode == "add") {
		return Instruction::add;
	} else if (opcode == "sub") {
		return Instruction::sub;
	} else if (opcode == "mul") {
		return Instruction::mul;
	} else if (opcode == "div") {
		return Instruction::div;
	} else if (opcode == "addi") {
		return Instruction::addi;
	} else if (opcode == "subi") {
		return Instruction::subi;
	} else if (opcode == "lw") {
		return Instruction::lw;
	} else if (opcode == "sw") {
		return Instruction::sw;
	} else if (opcode == "beq") {
		return Instruction::beq;
	} else if (opcode == "bne") {
		return Instruction::bne;
	} else if (opcode == "slt") {
		return Instruction::slt;
	} else if (opcode == "slti") {
		return Instruction::slti;
	} else if (opcode == "j") {
		return Instruction::j;
	} else if (opcode == "halt") {
		return Instruction::halt;
	} else {
		return Instruction::unknown;
	}
}