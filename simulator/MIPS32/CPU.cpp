#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"


bool CPU::execute(vector<Instruction> instructions, ISA::Address instructionBase, ISA::Address pc) {
	char rTypeResult = 0;

	for (std::vector<Instruction>::iterator inst_iter = instructions.begin(); inst_iter < instructions.end(); inst_iter++) {
		switch (inst_iter->getOpcode()) {
		case ISA::add:
			RTypeInstruction* pRInst = dynamic_cast<RTypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRd()] = gpr[pRInst->getRs()] + gpr[pRInst->getRt()];
		case ISA::sub:
			RTypeInstruction* pRInst = dynamic_cast<RTypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRd()] = gpr[pRInst->getRs()] - gpr[pRInst->getRt()];
		case ISA::mul:
			RTypeInstruction* pRInst = dynamic_cast<RTypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRd()] = gpr[pRInst->getRs()] - gpr[pRInst->getRt()];
		case ISA::div:
			RTypeInstruction* pRInst = dynamic_cast<RTypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRd()] = gpr[pRInst->getRs()] / gpr[pRInst->getRt()];
		case ISA::slt:
			RTypeInstruction* pRInst = dynamic_cast<RTypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRd()] = gpr[pRInst->getRs()] < gpr[pRInst->getRt()];
		case ISA::addi:
			ITypeInstruction* pIInst = dynamic_cast<ITypeInstruction*>(&*inst_iter);			
			gpr[pRInst->getRt()] = gpr[pIInst->getRs()] + gpr[pIInst->getImmediate()];
		case ISA::subi:
		case ISA::slti:
			//instruction = parseImmediateArithmeticInstruction(opcode, match[2]);
			break;
		case ISA::lw:
		case ISA::sw:
			//TODO use shay's private memory access methds.			
		case ISA::beq:
		case ISA::bne:
			//instruction = parseBranchInstruction(opcode, match[2]);
			break;
		case ISA::j:
			//instruction = parseJumpInstruction(opcode, match[2]);
			break;
		case ISA::halt:
			// Handle special instruction
			//instruction = new Instruction(opcode);
			break;
	default:
		// Unidentified instruction (bug? error?)
		//instruction = NULL;
		break;
	}
	return false;
}

int CPU::getInstructionsCount() const {
	return instructionsExecuted;
}

ISA::Register CPU::readMemory(ISA:Address address) {
	return ISA::reverseEndianity(((int*)memory)[address]);
}

void CPU::writeMemory(ISA::Register value) {
	((int*)memory)[address] = ISA::reverseEndianity(value);
}