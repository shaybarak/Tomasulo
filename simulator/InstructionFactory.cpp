#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "LiteralJTypeInstruction.h"
#include "SymbolicJTypeInstruction.h"
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
	// TODO extract methods out of this huge switch statement
	int rs, rt, rd;
	short immediate;
	int target;
	switch (opcode) {
	add:
	sub:
	mul:
	div:
	slt:
		// Handle R-Type instruction
		if (sscanf("$%d $%d $%d", &rs, &rt, &rd) != 3) {
			cerr << "Error parsing arguments in line " << line << endl;
			return NULL;
		}
		// TODO extract this to a method with ellipsis
		if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31 && 0 <= rd && rd <= 31)) {
			cerr << "Register index out of range in line " << line << endl;
			return NULL;
		}
		return new RTypeInstruction(opcode, rs, rt, rd);
	addi:
	subi:
	slti:
		// Handle I-Type arithmetic instruction
		if (sscanf("$%d $%d %hd", &rs, &rt, &immediate) != 3) {
			cerr << "Error parsing arguments in line " << line << endl;
			return NULL;
		}
		if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31)) {
			cerr << "Register index out of range in line " << line << endl;
			return NULL;
		}
		return new ITypeInstruction(opcode, rs, rt, immediate);
	lw:
	sw:
		// Handle I-Type memory instruction
		if (sscanf("$%d (%hd)$%d", &rs, &immediate, &rt) != 3) {
			cerr << "Error parsing arguments in line " << line << endl;
			return NULL;
		}
	beq:
	bne:
		// Handle I-Type branch instruction
		if (sscanf("$%d $%d %hd", &rs, &rt, &immediate) == 3) {
			return new ITypeInstruction(opcode, rs, rt, immediate);
		} else {
			// Literal target not identified, try symbolic target
			char symbolicTarget[21];
			if (sscanf("$%d $%d %20s", &rs, &rt, &symbolicTarget) != 3) {
				cerr << "Bad jump target in line " << line << endl;
				return NULL;
			}
			string symbol(symbolicTarget);
			map<string, int>::iterator symbolValue = symbols.find(symbol);
			if (symbolValue == symbols.end()) {
				cerr << "Unidentified symbol in line " << line << endl;
				return NULL;
			}
			return new ITypeInstruction(opcode, rs, rt, *symbolValue);
		}
	j:
		// Handle J-Type instruction
		if (sscanf("%d", &literalTarget) == 1) {
			if (!(-33554432 <= literalTarget && literalTarget <= 33554431)) {
				cerr << "Literal jump target out of range in line " << line << endl;
				return NULL;
			}
			return new JTypeInstruction(opcode, literalTarget);
		} else {
			// Literal target not identified, try symbolic target
			char symbolicTarget[21];
			if (sscanf("%20s", &symbolicTarget) != 1) {
				cerr << "Bad jump target in line " << line << endl;
				return NULL;
			}
			string symbol(symbolicTarget);
			map<string, int>::iterator symbolValue = symbols.find(symbol);
			if (symbolValue == symbols.end()) {
				cerr << "Unidentified symbol in line " << line << endl;
				return NULL;
			}
			return new JTypeInstruction(opcode, *symbolValue);
		}
	halt:
		// Handle special instruction
		return new SpecialInstruction(opcode);
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