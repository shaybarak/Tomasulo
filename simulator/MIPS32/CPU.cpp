#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"

bool CPU::execute(vector<Instruction>& instructions, int instructionBase, int pc) {
	bool halted = false;
	bool error = false;
	while (!error && !finished) {
		if ((pc - instructionBase < 0) || (pc - instructionBase >= instructions.size())) {
			cerr << "CPU exception: program counter out of range!" << endl;
			error = true;
			continue;
		}
		Instruction instruction& = instructions[pc - instructionBase];
		pc++;
		instructionsExecuted++;
		switch (instruction.getOpcode()) {
		case ISA::add:
			RTypeInstruction* rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			gpr[rtype->getRd()] = gpr[rtype->getRs()] + gpr[rtype->getRt()];
			break;
		case ISA::sub:
			RTypeInstruction* rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			gpr[rtype->getRd()] = gpr[rtype->getRs()] - gpr[rtype->getRt()];
			break;
		case ISA::mul:
			RTypeInstruction* rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			gpr[rtype->getRd()] = gpr[rtype->getRs()] * gpr[rtype->getRt()];
			break;
		case ISA::div:
			RTypeInstruction* rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			if (rtype->getRt() == 0) {
				cerr << "CPU exception: division by zero!" << endl;
				error = true;
				pc--;
				continue;
			}
			gpr[rtype->getRd()] = gpr[rtype->getRs()] / gpr[rtype->getRt()];
			break;
		case ISA::slt:
			RTypeInstruction* rtype = dynamic_cast<RTypeInstruction*>(&instruction);
			gpr[rtype->getRd()] = (gpr[rtype->getRs()] < gpr[rtype->getRt()]) ? 1 : 0;
			break;
		case ISA::addi:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			gpr[itype->getRt()] = gpr[itype->getRs()] + itype->getImmediate();
			break;
		case ISA::subi:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			gpr[itype->getRt()] = gpr[itype->getRs()] - itype->getImmediate();
			break;
		case ISA::slti:
			RTypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			gpr[itype->getRt()] = (gpr[itype->getRs()] < itype->getImmediate()) ? 1 : 0;
			break;
		case ISA::lw:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			int mem;
			if (!readMemory(itype->getRs() + itype->getImmediate(), &mem)) {
				cerr << "CPU exception: memory offset out of range!" << endl;
				error = true;
				pc--;
				continue;
			}
			gpr[itype->getRt()] = mem;
			break;
		case ISA::sw:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (!writeMemory(itype->getRs() + itype->getImmediate(), itype->getRt())) {
				cerr << "CPU exception: memory offset out of range!" << endl;
				error = true;
				pc--;
				continue;
			}
			break;
		case ISA::beq:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (itype->getRs() == itype->getRt()) {
				pc += itype->getImmediate();
			}
			break;
		case ISA::bne:
			ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(&instruction);
			if (itype->getRs() != itype->getRt()) {
				pc += itype->getImmediate();
			}
			break;
		case ISA::j:
			JTypeInstruction* jtype = dynamic_cast<JTypeInstruction*>(&instruction);
			pc = jtype->getTarget();
			break;
		case ISA::halt:
			finished = true;
			break;
		default:
			cerr << "CPU exception: invalid opcode!" << endl;
			error = true;
			pc--;
			instructionsExecuted--;
			break;
		}
		instructionsExecuted++;
	}
	return !error;
}

int CPU::getInstructionsCount() const {
	return instructionsExecuted;
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