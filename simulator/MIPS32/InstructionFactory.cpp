#include "InstructionFactory.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include "ISA.h"
#include "GPR.h"
#include <stdlib.h>

const regex InstructionFactory::labeledInstruction("^(\\s*\\w*\\s*:\\s*(\\w*)\\s*(.*))$");
const regex InstructionFactory::unlabeledInstruction("^(\\s*(\\w*)\\s*(.*))$");
const regex InstructionFactory::emptyLine("^\\s*$");

Instruction* InstructionFactory::parse(string& line) {
	// Skip empty lines
	if (regex_match(line, emptyLine)) {
		return NULL;
	}
	smatch match;
	// Attempt to match either labeled or unlabeled instructions
	// (we don't care about labels here)
	if (regex_search(line, match, labeledInstruction) ||
		regex_search(line, match, unlabeledInstruction)) {
	} else {
		cerr << "Cannot parse command line " << line << endl;
		return NULL;
	}
	ISA::Opcode opcode = ISA::toOpcode(match[2]);
	Instruction* instruction = NULL;
	switch (opcode) {
	case ISA::add:
	case ISA::sub:
	case ISA::mul:
	case ISA::div:
	case ISA::slt:
		instruction = parseRegisterArithmeticInstruction(match[1], opcode, match[3]);
		break;
	case ISA::addi:
	case ISA::subi:
	case ISA::slti:
		instruction = parseImmediateArithmeticInstruction(match[1], opcode, match[3]);
		break;
	case ISA::lw:
	case ISA::sw:
		instruction = parseMemoryInstruction(match[1], opcode, match[3]);
		break;
	case ISA::beq:
	case ISA::bne:
		instruction = parseBranchInstruction(match[1], opcode, match[3]);
		break;
	case ISA::j:
		instruction = parseJumpInstruction(match[1], opcode, match[3]);
		break;
	case ISA::halt:
		// Handle special instruction
		instruction = new Instruction(match[1].str(), pc, opcode);
		break;
	default:
		// Unidentified instruction (bug? error?)
		instruction = NULL;
		break;
	}
	if (instruction == NULL) {
		cerr << "Error while parsing " << line << endl;
		return NULL;
	} else {
		pc++;
		return instruction;
	}
}

RTypeInstruction* InstructionFactory::parseRegisterArithmeticInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const {
	int rs, rt, rd;
	if ((sscanf_s(arguments.c_str(), "$%d $%d $%d", &rd, &rs, &rt) != 3) &&
		(sscanf_s(arguments.c_str(), "$%d,$%d,$%d", &rd, &rs, &rt) != 3)) {
		return NULL;
	}
	if (!(GPR::isValid(rs) && GPR::isValid(rt) && GPR::isValid(rd))) {
		return NULL;
	}
	return new RTypeInstruction(inst, pc, opcode, rs, rt, rd);
}

ITypeInstruction* InstructionFactory::parseImmediateArithmeticInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const {
	int rs, rt;
	short immediate;
	if ((sscanf_s(arguments.c_str(), "$%d $%d %hd", &rt, &rs, &immediate) != 3) &&
		(sscanf_s(arguments.c_str(), "$%d,$%d,%hd", &rt, &rs, &immediate) != 3)){
		return NULL;
	}
	if (!(GPR::isValid(rs) && GPR::isValid(rt))) {
		return NULL;
	}
	return new ITypeInstruction(inst, pc, opcode, rs, rt, immediate);
}

ITypeInstruction* InstructionFactory::parseMemoryInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const {
	int rs, rt;
	short immediate;
	if ((sscanf_s(arguments.c_str(), "$%d (%hd)$%d", &rt, &immediate, &rs) != 3) && 
		(sscanf_s(arguments.c_str(), "$%d,(%hd)$%d", &rt, &immediate, &rs) != 3)) {
		return NULL;
	}
	return new ITypeInstruction(inst, pc, opcode, rs, rt, immediate);
}

ITypeInstruction* InstructionFactory::parseBranchInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const {
	int rs, rt;
	short immediate;
	if ((sscanf_s(arguments.c_str(), "$%d $%d %hd", &rs, &rt, &immediate) == 3) &&
		(sscanf_s(arguments.c_str(), "$%d,$%d,%hd", &rs, &rt, &immediate) == 3)){
		// Parsed a literal target
	} else {
		// Try to parse labeled target
		char labeledTarget[101];
		if (// Can't use sscanf_s with string.c_str() when %s is present due to VS CRT bug
			#pragma warning(disable:4996)
			(sscanf(arguments.c_str(), "$%d $%d %100s", &rs, &rt, labeledTarget) != 3) &&
			// Can't use sscanf_s with string.c_str() due to VS CRT bug
			#pragma warning(disable:4996)
			(sscanf(arguments.c_str(), "$%d,$%d,%100s", &rs, &rt, labeledTarget) != 3)){
			return NULL;
		}
		string label(labeledTarget);
		map<string, int>::const_iterator labelValue = labels.find(label);
		if (labelValue == labels.end()) {
			return NULL;
		}
		// Fix up; relative_target = absolute_target - (PC + 1)
		immediate = labelValue->second - (pc + 1);
	}
	if (!(GPR::isValid(rs) && GPR::isValid(rt))) {
		return NULL;
	}
	return new ITypeInstruction(inst, pc, opcode, rs, rt, immediate);
}

JTypeInstruction* InstructionFactory::parseJumpInstruction(const string& inst, ISA::Opcode opcode, const string& arguments) const {
	int literalTarget;
	if (sscanf_s(arguments.c_str(), "%d", &literalTarget) == 1) {
		// Jump target is 26 bits
		if (!(-33554432 <= literalTarget && literalTarget <= 33554431)) {
			return NULL;
		}
		return new JTypeInstruction(inst, pc, opcode, literalTarget);
	} else {
		// Literal target not identified, try labeled target
		char labeledTarget[101];
		if (sscanf(arguments.c_str(), "%100s", labeledTarget) != 1) {
			return NULL;
		}
		string label(labeledTarget);
		map<string, int>::const_iterator labelValue = labels.find(label);
		if (labelValue == labels.end()) {
			return NULL;
		}
		return new JTypeInstruction(inst, pc, opcode, labelValue->second);
	}
}
