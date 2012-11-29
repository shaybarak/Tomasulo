#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include <stdlib.h>

const regex InstructionFactory::labeledInstruction("^\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*)$");
const regex InstructionFactory::unlabeledInstruction("^\\s*(\\w*)\\s*(.*)$");

Instruction* InstructionFactory::parse(string& line, unsigned int address) const {
	// TODO use address for branch offset calculation
	smatch match;
	// Attempt to match either labeled or unlabeled instructions
	// (we don't care about labels here)
	if (regex_search(line, match, labeledInstruction) ||
		regex_search(line, match, unlabeledInstruction)) {
	} else {
		cerr << "Cannot parse command line " << line << endl;
		return NULL;
	}
	ISA::Opcode opcode = ISA::toOpcode(match[1]);
	Instruction* instruction = NULL;
	switch (opcode) {
	case ISA::add:
	case ISA::sub:
	case ISA::mul:
	case ISA::div:
	case ISA::slt:
		instruction = parseRegisterArithmeticInstruction(opcode, match[2]);
		break;
	case ISA::addi:
	case ISA::subi:
	case ISA::slti:
		instruction = parseImmediateArithmeticInstruction(opcode, match[2]);
		break;
	case ISA::lw:
	case ISA::sw:
		instruction = parseMemoryInstruction(opcode, match[2]);
		break;
	case ISA::beq:
	case ISA::bne:
		instruction = parseBranchInstruction(opcode, match[2]);
		break;
	case ISA::j:
		instruction = parseJumpInstruction(opcode, match[2]);
		break;
	case ISA::halt:
		// Handle special instruction
		instruction = new Instruction(opcode);
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

bool InstructionFactory::validateRegisterIndex(int index) {
	// ISA supports 32 registers
	return (0 <= index && index < (ISA::REG_COUNT));
}

RTypeInstruction* InstructionFactory::parseRegisterArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const {
	int rs, rt, rd;
	if ((sscanf_s(arguments.c_str(), "$%d $%d $%d", &rs, &rt, &rd) != 3) &&
		(sscanf_s(arguments.c_str(), "$%d, $%d, $%d", &rs, &rt, &rd) != 3)) {
		return NULL;
	}
	if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31 && 0 <= rd && rd <= 31)) {
		return NULL;
	}
	return new RTypeInstruction(opcode, rs, rt, rd);
}

ITypeInstruction* InstructionFactory::parseImmediateArithmeticInstruction(ISA::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if ((sscanf_s(arguments.c_str(), "$%d $%d %hd", &rs, &rt, &immediate) != 3) &&
		(sscanf_s(arguments.c_str(), "$%d, $%d, %hd", &rs, &rt, &immediate) != 3)){
		return NULL;
	}
	if (!(0 <= rs && rs <= 31 && 0 <= rt && rt <= 31)) {
		return NULL;
	}
	return new ITypeInstruction(opcode, rs, rt, immediate);
}

ITypeInstruction* InstructionFactory::parseMemoryInstruction(ISA::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if ((sscanf_s(arguments.c_str(), "$%d (%hd)$%d", &rs, &immediate, &rt) != 3) && 
		(sscanf_s(arguments.c_str(), "$%d, (%hd)$%d", &rs, &immediate, &rt) != 3)) {
		return NULL;
	}
	return new ITypeInstruction(opcode, rs, rt, immediate);
}

ITypeInstruction* InstructionFactory::parseBranchInstruction(ISA::Opcode opcode, const string& arguments) const {
	int rs, rt, immediate;
	if ((sscanf_s(arguments.c_str(), "$%d $%d %hd", &rs, &rt, &immediate) == 3) &&
		(sscanf_s(arguments.c_str(), "$%d, $%d, %hd", &rs, &rt, &immediate) == 3)){
		return new ITypeInstruction(opcode, rs, rt, immediate);
	} else {
		// Literal target not identified, try labelled target
		char labelledTarget[21];
		if ((sscanf_s(arguments.c_str(), "$%d $%d %20s", &rs, &rt, &labelledTarget) != 3) &&
			(sscanf_s(arguments.c_str(), "$%d, $%d, %20s", &rs, &rt, &labelledTarget) != 3)){
			return NULL;
		}
		string label(labelledTarget);
		map<string, unsigned int>::const_iterator labelValue = labels.find(label);
		if (labelValue == labels.end()) {
			return NULL;
		}
		return new ITypeInstruction(opcode, rs, rt, labelValue->second);
	}
}

JTypeInstruction* InstructionFactory::parseJumpInstruction(ISA::Opcode opcode, const string& arguments) const {
	int literalTarget;
	if (sscanf_s(arguments.c_str(), "%d", &literalTarget) == 1) {
		// Jump target is 26 bits
		if (!(-33554432 <= literalTarget && literalTarget <= 33554431)) {
			return NULL;
		}
		return new JTypeInstruction(opcode, literalTarget);
	} else {
		// Literal target not identified, try labelled target
		char labelledTarget[101];
		if (sscanf(arguments.c_str(), "%100s", labelledTarget) != 1) {
			return NULL;
		}
		string label(labelledTarget);
		map<string, unsigned int>::const_iterator labelValue = labels.find(label);
		if (labelValue == labels.end()) {
			return NULL;
		}
		return new JTypeInstruction(opcode, labelValue->second);
	}
}