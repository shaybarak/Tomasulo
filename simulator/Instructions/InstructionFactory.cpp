#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include "SpecialInstruction.h"
#include <stdlib.h>

regex labeledInstruction("^\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*)$");
regex unlabeledInstruction("^\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*)$");

Instruction* InstructionFactory::parse(string& line) const {
	smatch match;
	// Attempt to match either labeled or unlabeled instructions
	// (we don't care about labels here)
	if (regex_search(line, match, labeledInstruction) ||
		regex_search(line, match, unlabeledInstruction)) {
	} else {
		cerr << "Cannot parse command line " << line << endl;
		return NULL;
	}
	Instruction::Opcode opcode = toOpcode(match[1]);
	Instruction* instruction = NULL;
	switch (opcode) {
	add:
	sub:
	mul:
	div:
	slt:
		instruction = parseRegisterArithmeticInstruction(opcode, match[2]);
		break;
	addi:
	subi:
	slti:
		instruction = parseImmediateArithmeticInstruction(opcode, match[2]);
		break;
	lw:
	sw:
		instruction = parseMemoryInstruction(opcode, match[2]);
		break;
	beq:
	bne:
		instruction = parseBranchInstruction(opcode, match[2]);
		break;
	j:
		instruction = parseJumpInstruction(opcode, match[2]);
		break;
	halt:
		// Handle special instruction
		instruction = new SpecialInstruction(opcode);
		break;
	default:
		// Unidentified instruction (bug? error?)
		instruction = NULL;
		break;
	}
	if (instruction == NULL) {
		cerr << "Error while parsing " << line << endl;
	}
	return instruction;
}

static Instruction::Opcode toOpcode(string& opcode) {
	// Convert opcode to lowercase for comparison
	string opcodeLowered;
	transform(opcode.begin(), opcode.end(), opcodeLowered.begin(), tolower);
	if (opcodeLowered == "add") {
		return Instruction::add;
	} else if (opcodeLowered == "sub") {
		return Instruction::sub;
	} else if (opcodeLowered == "mul") {
		return Instruction::mul;
	} else if (opcodeLowered == "div") {
		return Instruction::div;
	} else if (opcodeLowered == "addi") {
		return Instruction::addi;
	} else if (opcodeLowered == "subi") {
		return Instruction::subi;
	} else if (opcodeLowered == "lw") {
		return Instruction::lw;
	} else if (opcodeLowered == "sw") {
		return Instruction::sw;
	} else if (opcodeLowered == "beq") {
		return Instruction::beq;
	} else if (opcodeLowered == "bne") {
		return Instruction::bne;
	} else if (opcodeLowered == "slt") {
		return Instruction::slt;
	} else if (opcodeLowered == "slti") {
		return Instruction::slti;
	} else if (opcodeLowered == "j") {
		return Instruction::j;
	} else if (opcodeLowered == "halt") {
		return Instruction::halt;
	} else {
		return Instruction::unknown;
	}
}

bool InstructionFactory::validateRegisterIndex(int index) {
	// ISA supports 32 registers
	return (0 <= index && index <= 31);
}

Instruction* InstructionFactory::parseRegisterArithmeticInstruction(Instruction::Opcode opcode, const string& arguments) const {
	int rs, rt, rd;
	if (sscanf(arguments.c_str(), "$%d $%d $%d", &rs, &rt, &rd) != 3) {
		return NULL;
	}
	if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31 && 0 <= rd && rd <= 31)) {
		return NULL;
	}
	return new RTypeInstruction(opcode, rs, rt, rd);
}

Instruction* InstructionFactory::parseImmediateArithmeticInstruction(Instruction::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if (sscanf(arguments.c_str(), "$%d $%d %hd", &rs, &rt, &immediate) != 3) {
		return NULL;
	}
	if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31)) {
		return NULL;
	}
	return new ITypeInstruction(opcode, rs, rt, immediate);
}

Instruction* InstructionFactory::parseMemoryInstruction(Instruction::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if (sscanf(arguments.c_str(), "$%d (%hd)$%d", &rs, &immediate, &rt) != 3) {
		return NULL;
	}
	return new ITypeInstruction(opcode, rs, rt, immediate);
}

Instruction* InstructionFactory::parseBranchInstruction(Instruction::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if (sscanf(arguments.c_str(), "$%d $%d %hd", &rs, &rt, &immediate) == 3) {
		return new ITypeInstruction(opcode, rs, rt, immediate);
	} else {
		// Literal target not identified, try symbolic target
		char symbolicTarget[21];
		if (sscanf(arguments.c_str(), "$%d $%d %20s", &rs, &rt, &symbolicTarget) != 3) {
			return NULL;
		}
		string symbol(symbolicTarget);
		map<string, int>::iterator symbolValue = symbols.find(symbol);
		if (symbolValue == symbols.end()) {
			return NULL;
		}
		return new ITypeInstruction(opcode, rs, rt, symbolValue->second);
	}
}

Instruction* InstructionFactory::parseJumpInstruction(Instruction::Opcode opcode, const string& arguments) const {
	int literalTarget;
	if (sscanf(arguments.c_str(), "%d", &literalTarget) == 1) {
		// Jump target is 26 bits
		if (!(-33554432 <= literalTarget && literalTarget <= 33554431)) {
			return NULL;
		}
		return new JTypeInstruction(opcode, literalTarget);
	} else {
		// Literal target not identified, try symbolic target
		char symbolicTarget[21];
		if (sscanf(arguments.c_str(), "%20s", &symbolicTarget) != 1) {
			return NULL;
		}
		string symbol(symbolicTarget);
		map<string, int>::const_iterator symbolValue = symbols.find(symbol);
		if (symbolValue == symbols.end()) {
			return NULL;
		}
		return new JTypeInstruction(opcode, symbolValue->second);
	}
}