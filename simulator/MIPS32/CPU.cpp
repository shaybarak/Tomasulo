#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include <assert.h>

void CPU::loadProgram(vector<Instruction*>* instructions, int pc) {
	this->instructions = instructions;
	this->pc = pc;
}

void CPU::runOnce() {
	// Don't execute if halted or if no instructions present
	if (halted || (instructions == NULL)) {
		return;
	}
	// Validate program counter
	assert(pc >= 0);
	#pragma warning(disable:4018)  // pc is known to be non-negative at this time
	assert(pc < instructions->size());
	readInstruction();
	pc++;
	execute(instructions->at(pc-1));
	instructionsCommitted++;
}

void CPU::readInstruction() {
	int instruction;
	// Read from instruction memory
	now = instructionMemory->read(now, pc * sizeof(int), instruction);
	memoryAccessCount++;
	// Verify that instruction was correctly read from memory
	assert(pc == instruction);
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
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] + (*gpr)[rtype->getRt()];
		break;
	case ISA::sub:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] - (*gpr)[rtype->getRt()];
		break;
	case ISA::mul:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] * (*gpr)[rtype->getRt()];
		break;
	case ISA::div:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		if ((*gpr)[rtype->getRt()] == 0) {
			cerr << "CPU exception: division by zero!" << endl;
			halted = true;
			break;
		}
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] / (*gpr)[rtype->getRt()];
		break;
	case ISA::slt:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = ((*gpr)[rtype->getRs()] < (*gpr)[rtype->getRt()]) ? 1 : 0;
		break;
	case ISA::addi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] + itype->getImmediate();
		break;
	case ISA::subi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] - itype->getImmediate();
		break;
	case ISA::slti:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = ((*gpr)[itype->getRs()] < itype->getImmediate()) ? 1 : 0;
		break;
	case ISA::lw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = readData((*gpr)[itype->getRs()] + itype->getImmediate());
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		writeData((*gpr)[itype->getRs()] + itype->getImmediate(), (*gpr)[itype->getRt()]);
		break;
	case ISA::beq:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()] == (*gpr)[itype->getRt()]) {
			pc += itype->getImmediate();
		}
		break;
	case ISA::bne:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()] != (*gpr)[itype->getRt()]) {
			pc += itype->getImmediate();
		}
		break;
	case ISA::j:
		jtype = dynamic_cast<JTypeInstruction*>(instruction);
		pc = jtype->getTarget();
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
