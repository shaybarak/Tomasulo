#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

void CPU::runOnce() {
	// Try to execute any instructions that are in reservation and ready
	execute();
	// Issue the next instruction to a reservation station, if possible
	issue();
	// If any instructions have finished execution, write them on the CDB
	writeCdb();
	now++;
}

ReservationStation* CPU::getRs(ISA::Opcode opcode) {
	switch (opcode) {
	
	case ISA::mul:
	case ISA::div:
		return rsMulDiv;
	
	case ISA::add:
	case ISA::addi:
	case ISA::sub:
	case ISA::subi:
	case ISA::slt:
	case ISA::slti:
		return rsAddSub;
	
	case ISA::lw:
		return rsLoad;	
	case ISA::sw:
		return rsStore;

	default:
		assert(false);
		break; 
	}
}

void CPU::addInstructionToRs(ReservationStation* rs, int index, Instruction* instruction) {

	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;

	ISA::Tag newTag;

	(*rs)[index].busy = true;
	(*rs)[index].instruction = instruction;
	(*rs)[index].timeIssued = now;
	(*rs)[index].timeWriteCDB = -1;

	newTag.index = index;
	newTag.type = rs->getTagType();
	newTag.valid = true;

	switch (instruction->getOpcode()) {
	
	//RType
	case ISA::add:
	case ISA::sub:
	case ISA::mul:
	case ISA::div: 
	case ISA::slt:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		GPR::Reg jReg = (*gpr)[rtype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		GPR::Reg kReg = (*gpr)[rtype->getRt()];
		(*rs)[index].vk = kReg.value;
		(*rs)[index].qk = kReg.tag;	
		(*gpr)[rtype->getRd()].tag = newTag;
		break;

	//IType
	case ISA::addi:
	case ISA::subi:
	case ISA::slti:
	case ISA::lw:
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		GPR::Reg jReg = (*gpr)[itype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		(*rs)[index].vk = itype->getImmediate();
		(*rs)[index].qk.valid = false;
		(*gpr)[itype->getRt()].tag = newTag;
		break;

	default:
		assert(false);
		break;
	}
}

//TODO multiple issue?
bool CPU::issue(Instruction* instruction) {
	ReservationStation* chosenRs = getRs(instruction->getOpcode());
	int index = chosenRs->getFreeIndex();
	if (index == -1) {
		return false;
	}
	addInstructionToRs(chosenRs,index, instruction);
}

void CPU::fetch(bool issued) {
	if (issued) {
		instructionQueue->popNextInstruction();
	}
	if (instructionQueue->tryFetch(now)) {
		memoryAccessCount++;
	}
}

bool CPU::writeCDB(ReservationStation* rs) {
	bool cdbOccupied = false;
	ReservationStation::Entry entry;
	for (int index = 0; index < rs->getSize(); index++) {
		entry = (*rs)[index];
		if (entry.timeWriteCDB >= now) {
			cdbValue = computeValue(rs, index);
			cdbTag.index = index;
			cdbTag.type = rs->getTagType();
			cdbTag.valid = true;
			(*rs)[index].busy = false;
			return true;
		}
	}
}

void CPU::writeCDB() {
	bool cdbOccupied = false;
	cdbOccupied = writeCDB(rsAddSub);	
	if (!cdbOccupied) {
		cdbOccupied = writeCDB(rsMulDiv);
	}

	//TODO load/store
	if (cdbOccupied) {
		gpr->updateTags(cdbTag, cdbValue);
		rsAddSub->updateTags(cdbTag, cdbValue);
		rsMulDiv->updateTags(cdbTag, cdbValue);
		rsLoad->updateTags(cdbTag, cdbValue);
		rsStore->updateTags(cdbTag, cdbValue);
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


int CPU::computeValue(ReservationStation* rs, int index) {
	ReservationStation::Entry entry = (*rs)[index];
	switch (entry.instruction->getOpcode()) {
	case ISA::add:
	case ISA::addi:
		return entry.vj + entry.vk;
	case ISA::sub:
	case ISA::subi:
		return entry.vj - entry.vk;
	case ISA::slt:
	case ISA::slti:
		return entry.vj < entry.vk ? 1 : 0;
	case ISA::mul:
		return entry.vj * entry.vk;
	case ISA::div:
		if (entry.vk == 0) {
			cerr << "CPU exception: division by zero!" << endl;
			halted = true;
			return 0;
		}
		return entry.vj / entry.vk;
	case ISA::lw:
	case ISA::sw:
		//TODO
	default:
		// Unexpected instruction!
		assert(false);
		break;
	}
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
		} else {
			instructionQueue->setPc(instructionQueue->getPc());
		}
		break;
	case ISA::bne:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()].value != (*gpr)[itype->getRt()].value) {
			instructionQueue->setPc(instructionQueue->getPc() + itype->getImmediate());
		} else {
			instructionQueue->setPc(instructionQueue->getPc());
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
