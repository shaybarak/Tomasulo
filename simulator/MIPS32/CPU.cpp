#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

void CPU::runOnce() {
	// Don't execute if halted or if no instructions present
	if (halted || (instructions == NULL)) {
		return;
	}
	//readInstruction();
	Instruction* nextInstruction = decode();
	fetch();
	if (nextInstruction != NULL) {
		execute(nextInstruction); // TODO change this to issue, execute later
		instructionsCommitted++;  // TODO count instructions committed elsewhere
	}
	now++;
}

void CPU::fetch() {
	if (instructionQueue->tryReadNewInstruction(now)) {
		memoryAccessCount++;
	}
}

Instruction* CPU::decode() {
	return instructionQueue->tryGetNextInstruction(now);
}

int CPU::readData(int address) {
	// Verify address is within range
	assert(address >= 0);
	assert(address < ISA::DATA_SEG_SIZE);
	// Read from data memory
	int data;
	now = dataMemory->read(now, address, data);
	memoryAccessCount++;
	return data;
}

void CPU::writeData(int address, int data) {
	// Verify address is within range
	assert(address >= 0);
	assert(address < ISA::DATA_SEG_SIZE);
	// Write to data memory
	now = dataMemory->write(now, address, data);
	memoryAccessCount++;
}

void CPU::execute(Instruction* instruction) {
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	switch (instruction->getOpcode()) {
	case ISA::add:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()].value = (*gpr)[rtype->getRs()].value + (*gpr)[rtype->getRt()].value;
		break;
	case ISA::sub:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()].value = (*gpr)[rtype->getRs()].value - (*gpr)[rtype->getRt()].value;
		break;
	case ISA::mul:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()].value = (*gpr)[rtype->getRs()].value * (*gpr)[rtype->getRt()].value;
		break;
	case ISA::div:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		if ((*gpr)[rtype->getRt()].value == 0) {
			cerr << "CPU exception: division by zero!" << endl;
			halted = true;
			break;
		}
		(*gpr)[rtype->getRd()].value = (*gpr)[rtype->getRs()].value / (*gpr)[rtype->getRt()].value;
		break;
	case ISA::slt:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()].value = ((*gpr)[rtype->getRs()].value < (*gpr)[rtype->getRt()].value) ? 1 : 0;
		break;
	case ISA::addi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()].value = (*gpr)[itype->getRs()].value + itype->getImmediate();
		break;
	case ISA::subi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()].value = (*gpr)[itype->getRs()].value - itype->getImmediate();
		break;
	case ISA::slti:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()].value = ((*gpr)[itype->getRs()].value < itype->getImmediate()) ? 1 : 0;
		break;
	case ISA::lw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()].value = readData((*gpr)[itype->getRs()].value + itype->getImmediate());
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		writeData((*gpr)[itype->getRs()].value + itype->getImmediate(), (*gpr)[itype->getRt()].value);
		break;
	case ISA::beq:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()].value == (*gpr)[itype->getRt()].value) {
			instructionQueue->setPc(instructionQueue->getPc() + itype->getImmediate());
		}
		break;
	case ISA::bne:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()].value != (*gpr)[itype->getRt()].value) {
			instructionQueue->setPc(instructionQueue->getPc() + itype->getImmediate());
		}
		break;
	case ISA::j:
		// No handling is required since instruction queue takes care of jumps
		break;
	case ISA::halt:
		halted = true;
		break;
	default:
		// Unexpected instruction!
		assert(false);
		break;
	}
}
