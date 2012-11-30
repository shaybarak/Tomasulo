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
			if 
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
			break;
		case ISA::sw:
			break;
		case ISA::beq:
			break;
		case ISA::bne:
			break;
		case ISA::j:
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

int CPU::readMemory(int address) {
	return *(int*)(&memory[address]);
}

void CPU::writeMemory(int address, int value) {
	*(int*)(&memory[address]) = value;
}