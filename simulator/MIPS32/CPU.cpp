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

bool CPU::hasPendingInstructions() {
	return (rsAddSub->hasPendingInstructions() || 
		rsMulDiv->hasPendingInstructions() ||
		rsLoad->hasPendingInstructions() || rsStore->hasPendingInstructions());
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
		return NULL; 
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

	GPR::Reg jReg;
	GPR::Reg kReg;

	switch (instruction->getOpcode()) {
	
	//RType
	case ISA::add:
	case ISA::sub:
	case ISA::mul:
	case ISA::div: 
	case ISA::slt:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		jReg = (*gpr)[rtype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		kReg = (*gpr)[rtype->getRt()];
		(*rs)[index].vk = kReg.value;
		(*rs)[index].qk = kReg.tag;	
		(*gpr)[rtype->getRd()].tag = newTag;
		break;

	//IType
	case ISA::addi:
	case ISA::subi:
	case ISA::slti:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		jReg = (*gpr)[itype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		(*rs)[index].vk = itype->getImmediate();
		(*rs)[index].qk.valid = false;
		(*gpr)[itype->getRt()].tag = newTag;
		break;

	// Special handling for memory instructions
	case ISA::lw:
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		jReg = (*gpr)[itype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		kReg = (*gpr)[itype->getRt()];
		(*rs)[index].vj = kReg.value;
		(*rs)[index].qj = kReg.tag;
		break;


	// Special handling for branches
	case ISA::beq:
	case ISA::bne:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		jReg = (*gpr)[rtype->getRs()];
		(*rs)[index].vj = jReg.value;
		(*rs)[index].qj = jReg.tag;
		kReg = (*gpr)[rtype->getRt()];
		(*rs)[index].vk = kReg.value;
		(*rs)[index].qk = kReg.tag;	
		(*gpr)[rtype->getRd()].tag = newTag;
		break;

	case ISA::j:
		// No handling is required since instruction queue takes care of jumps
		break;

	default:
		assert(false);
		break;
	}
}

bool CPU::issue() {
	Instruction* instruction = instructionQueue->tryGetNextInstruction(now);
	if (instruction == NULL) {
		// No pending instruction, fetch next and retry issuing next time
		if (instructionQueue->tryFetch(now)) {
			memoryAccessCount++;
		}
		return false;
	}

	ReservationStation* chosenRs = getRs(instruction->getOpcode());
	int index = chosenRs->getFreeIndex();
	if (index == -1) {
		// Reservation station is busy
		return false;
	}
	// Issue to reservation station
	addInstructionToRs(chosenRs, index, instruction);
	instructionQueue->popNextInstruction();
	return true;
}

void CPU::executeLoad() {
	int index = rsLoad->findIndexToExecute(now);
	if (index != -1) {
		int address = (*rsLoad)[index].vj + 
			dynamic_cast<ITypeInstruction*>((*rsLoad)[index].instruction)->getImmediate();
		assert(address >= 0);
		assert(address < ISA::DATA_SEG_SIZE);
		int data;
		(*rsLoad)[index].timeWriteCDB = dataMemory->read(now + 1, address, data);
		(*rsLoad)[index].memValue = data;
		memoryAccessCount++;
	}
}

void CPU::executeStore() {
	int index = rsStore->findIndexToExecute(now);
	if (index != -1) {
		int address = (*rsLoad)[index].vj + 
			dynamic_cast<ITypeInstruction*>((*rsStore)[index].instruction)->getImmediate();
		assert(address >= 0);
		assert(address < ISA::DATA_SEG_SIZE);
		(*rsLoad)[index].timeWriteCDB = dataMemory->write(now + 1, address, (*rsLoad)[index].vk);
		memoryAccessCount++;
	}
}

bool CPU::execute() {
	//TODO support multiple executions for addsub
	rsAddSub->execute(now);
	rsMulDiv->execute(now);
	
	//TODO dataforward from store units, coherency etc.
	executeLoad();
	executeStore();

	return false;
}

bool CPU::writeCdb(ReservationStation* rs) {
	bool cdbOccupied = false;
	ReservationStation::Entry entry;
	for (int index = 0; index < rs->getSize(); index++) {
		entry = (*rs)[index];
		if ((entry.busy) && (entry.timeWriteCDB <= now)) {
			bool cdbWrite;
			(*rs)[index].busy = false;
			switch (entry.instruction->getOpcode()) {
			// Special handling for instructions that don't produce a value
			case ISA::bne:
				if (entry.vj == entry.vk) {
					ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(entry.instruction);
					instructionQueue->setPc(itype->getImmediate());
				} else {
					instructionQueue->setPc(instructionQueue->getPc() + 1);
				}
				cdbWrite = false;
			case ISA::beq:
				if (entry.vj != entry.vk) {
					ITypeInstruction* itype = dynamic_cast<ITypeInstruction*>(entry.instruction);
					instructionQueue->setPc(itype->getImmediate());
				} else {
					instructionQueue->setPc(instructionQueue->getPc() + 1);
				}
				cdbWrite = false;
			default:
				// All other instructions produce a value
				cdbValue = computeValue(rs, index);
				cdbTag.index = index;
				cdbTag.type = rs->getTagType();
				cdbTag.valid = true;
				cdbWrite = true;
			}
			instructionsCommitted++;
			trace->write((*rs)[index].instruction->toString(), 
				(*rs)[index].timeIssued,
				(*rs)[index].timeWriteCDB - rs->getDelay(),
				(*rs)[index].timeWriteCDB,
				now);
			return cdbWrite;
		}
	}
	return false;
}

void CPU::writeCdb() {
	bool cdbOccupied = false;
	cdbOccupied = writeCdb(rsAddSub);	
	if (!cdbOccupied) {
		cdbOccupied = writeCdb(rsMulDiv);
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
	case ISA::j:
		// Not expecting jump instructions
	default:
		// Unexpected instruction!
		assert(false);
		return 0;
	}
}