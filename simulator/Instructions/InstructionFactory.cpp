#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "LiteralJTypeInstruction.h"
#include "SymbolicJTypeInstruction.h"
#include "SpecialInstruction.h"
#include "UnknownInstruction.h"

Instruction* parseInstruction(string& line) {
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
	switch (opcode) {
	add:
	sub:
	mul:
	div:
	slt:
		return parseRegisterArithmeticInstruction(opcode, match[2]);
	addi:
	subi:
	slti:
		return parseImmediateArithmeticInstruction(opcode, match[2]);
	lw:
	sw:
		return parseMemoryInstruction(opcode, match[2]);
	beq:
	bne:
		return parseBranchInstruction(opcode, match[2]);
	j:
		return parseJumpInstruction(opcode, match[2]);
	halt:
		// Handle special instruction
		return new SpecialInstruction(opcode);
	default:
		// Unidentified instruction (bug? error?)
		return new UnknownInstruction();
	}
	/*switch (opcode) {
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
	}*/
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

Instruction* parseRegisterArithmeticInstruction(Instruction::Opcode opcode, string& arguments) {
	return NULL;
}

Instruction* parseImmediateArithmeticInstruction(Instruction::Opcode opcode, string& arguments) {
	return NULL;
}

Instruction* parseMemoryInstruction(Instruction::Opcode opcode, string& arguments) {
	return NULL;
}

Instruction* parseBranchInstruction(Instruction::Opcode opcode, string& arguments) {
	return NULL;
}

Instruction* parseJumpInstruction(Instruction::Opcode opcode, string& arguments) {
	return NULL;
}