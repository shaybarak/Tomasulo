#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"

bool CPU::execute(vector<Instruction>& instructions, int instructionBase, int pc) {
	bool halted = false;
	bool error = false;
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	while (!error && !halted) {
		if ((pc - instructionBase < 0) ||
			// (pc-instructionBase) is known to be non-negative at this time
			#pragma warning(disable:4018)
			(pc - instructionBase >= instructions.size())) {
			cerr << "CPU exception: program counter out of range!" << endl;
			error = true;
			continue;
		}
		Instruction& instruction = instructions[pc - instructionBase];
		pc++;
		instructionsCommitted++;
		executionTime++;
		switch (instruction.getOpcode()) {
		case ISA::add:
			rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] + (*gpr)[rtype->getRt()];
			break;
		case ISA::sub:
			rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] - (*gpr)[rtype->getRt()];
			break;
		case ISA::mul:
			rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] * (*gpr)[rtype->getRt()];
			break;
		case ISA::div:
			rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			if (rtype->getRt() == 0) {
				cerr << "CPU exception: division by zero!" << endl;
				error = true;
				pc--;
				instructionsCommitted--;
				continue;
			}
			(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] / (*gpr)[rtype->getRt()];
			break;
		case ISA::slt:
			rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			(*gpr)[rtype->getRd()] = ((*gpr)[rtype->getRs()] < (*gpr)[rtype->getRt()]) ? 1 : 0;
			break;
		case ISA::addi:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] + itype->getImmediate();
			break;
		case ISA::subi:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] - itype->getImmediate();
			break;
		case ISA::slti:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			(*gpr)[itype->getRt()] = ((*gpr)[itype->getRs()] < itype->getImmediate()) ? 1 : 0;
			break;
		case ISA::lw:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			int mem;
			if (!readMemory(itype->getRs() + itype->getImmediate(), &mem)) {
				cerr << "CPU exception: memory offset out of range!" << endl;
				error = true;
				pc--;
				instructionsCommitted--;
				continue;
			}
			(*gpr)[itype->getRt()] = mem;
			break;
		case ISA::sw:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (!writeMemory(itype->getRs() + itype->getImmediate(), itype->getRt())) {
				cerr << "CPU exception: memory offset out of range!" << endl;
				error = true;
				pc--;
				instructionsCommitted--;
				continue;
			}
			break;
		case ISA::beq:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (itype->getRs() == itype->getRt()) {
				pc += itype->getImmediate();
			}
			break;
		case ISA::bne:
			itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (itype->getRs() != itype->getRt()) {
				pc += itype->getImmediate();
			}
			break;
		case ISA::j:
			jtype = dynamic_cast<JTypeInstruction*>(&instruction);
			pc = jtype->getTarget();
			break;
		case ISA::halt:
			halted = true;
			break;
		default:
			cerr << "CPU exception: invalid opcode!" << endl;
			error = true;
			pc--;
			instructionsCommitted--;
			break;
		}
	}
	// Execution succeeded if no error occurred
	return !error;
}

int CPU::getInstructionsCommitted() const {
	return instructionsCommitted;
}

bool CPU::readMemory(int address, int* value) {
	if ((address < 0) || (address >= memorySize)) {
		return false;
	}
	*value = *(int*)(&memory[address]);
	return true;
}

bool CPU::writeMemory(int address, int value) {
	if ((address < 0) || (address >= memorySize)) {
		return false;
	}
	*(int*)(&memory[address]) = value;
	return true;
}