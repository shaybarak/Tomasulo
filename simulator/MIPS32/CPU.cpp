#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"

void CPU::loadProgram(vector<Instruction*>* instructions, int instructionsBase, int pc) {
	this->instructions = instructions;
	this->instructionsBase = instructionsBase;
	this->pc = pc;
	halted = false;
}

void CPU::onTick(int now) {
	this->now = now;
	cycles++;
	// Don't execute if halted
	if (halted) {
		return;
	}

	if (dataReadStall) {  // Stalled on data read
		int address;
		int data;
		if (!nextMemoryLevel->getReadResponse(&address, &data, now)) {
			// Read did not return yet
			return;
		}
		dataReadStall = false;
		execute(pcToInstructionIndex(pc), data);

	} else if (instructionReadStall) {  // Stalled on instruction read
		int address;
		int instruction;
		if (!nextMemoryLevel->getReadResponse(&address, &instruction, now)) {
			// Read did not return yet
			return;
		}
		// Verify that instruction was read correctly
		if (instruction != pcToInstructionIndex(pc)) {
			cerr << "CPU exception: illegal opcode " << instruction << "!" << endl;
			halted = true;
			return;
		}
		instructionReadStall = false;
		execute(pcToInstructionIndex(pc));
	}

	// If not ready for next instruction
	if (dataReadStall) {
		return;
	}

	if (!isValidInstructionAddress(pcToMemoryOffset(pc))) {
		cerr << "CPU exception: program counter out of range! PC=" << pc << endl;
		halted = true;
		return;
	}
	// Read next instruction
	nextMemoryLevel->requestRead(pcToMemoryOffset(pc), now);
	memoryAccessCount++;
	instructionReadStall = true;
}

void CPU::execute(int instructionIndex) {
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	Instruction* instruction = instructions->at(instructionIndex);
	int address, data;
	switch (instruction->getOpcode()) {
	case ISA::add:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] + (*gpr)[rtype->getRt()];
		pc++;
		instructionsCommitted++;
		break;
	case ISA::sub:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] - (*gpr)[rtype->getRt()];
		pc++;
		instructionsCommitted++;
		break;
	case ISA::mul:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] * (*gpr)[rtype->getRt()];
		pc++;
		instructionsCommitted++;
		break;
	case ISA::div:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		if ((*gpr)[rtype->getRt()] == 0) {
			cerr << "CPU exception: division by zero!" << endl;
			halted = true;
			break;
		}
		(*gpr)[rtype->getRd()] = (*gpr)[rtype->getRs()] / (*gpr)[rtype->getRt()];
		pc++;
		instructionsCommitted++;
		break;
	case ISA::slt:
		rtype = dynamic_cast<RTypeInstruction*>(instruction);
		(*gpr)[rtype->getRd()] = ((*gpr)[rtype->getRs()] < (*gpr)[rtype->getRt()]) ? 1 : 0;
		pc++;
		instructionsCommitted++;
		break;
	case ISA::addi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] + itype->getImmediate();
		pc++;
		instructionsCommitted++;
		break;
	case ISA::subi:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = (*gpr)[itype->getRs()] - itype->getImmediate();
		pc++;
		instructionsCommitted++;
		break;
	case ISA::slti:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = ((*gpr)[itype->getRs()] < itype->getImmediate()) ? 1 : 0;
		pc++;
		instructionsCommitted++;
		break;
	case ISA::lw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		address = (*gpr)[itype->getRs()] + itype->getImmediate();
		if (!isValidMemoryAddress(address)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			halted = true;
			break;
		}
		nextMemoryLevel->requestRead(address, now);
		memoryAccessCount++;
		// Note: stalled, don't advance PC and don't count instruction as committed
		dataReadStall = true;
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		address = (*gpr)[itype->getRs()] + itype->getImmediate();
		data = (*gpr)[itype->getRt()];
		if (!isValidMemoryAddress(address)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			halted = true;
			break;
		}
		nextMemoryLevel->requestWrite(address, data, now);
		memoryAccessCount++;
		// TODO do write operations stall?
		pc++;
		instructionsCommitted++;
		break;
	case ISA::beq:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()] == (*gpr)[itype->getRt()]) {
			pc += itype->getImmediate() + 1;
		} else {
			pc++;
		}
		instructionsCommitted++;
		break;
	case ISA::bne:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()] != (*gpr)[itype->getRt()]) {
			pc += itype->getImmediate() + 1;
		} else {
			pc++;
		}
		instructionsCommitted++;
		break;
	case ISA::j:
		jtype = dynamic_cast<JTypeInstruction*>(instruction);
		pc = jtype->getTarget();
		instructionsCommitted++;
		break;
	case ISA::halt:
		halted = true;
		instructionsCommitted++;
		break;
	default:
		cerr << "CPU exception: invalid opcode!" << endl;
		halted = true;
		break;
	}
}

void CPU::execute(int instructionIndex, int data) {
	ITypeInstruction* itype = NULL;
	Instruction* instruction = instructions->at(pc - instructionsBase);
	// Note: switch only implements lw because other instruction types are not expected
	switch (instruction->getOpcode()) {
	case ISA::lw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		(*gpr)[itype->getRt()] = data;
		pc++;
		instructionsCommitted++;
		break;
	default:
		cerr << "CPU exception: invalid opcode!" << endl;
		halted = true;
		break;
	}
}

bool CPU::isValidMemoryAddress(int address) {
	return (address >= 0) && (address < memorySize);
}

bool CPU::isValidInstructionAddress(int address) {
	if (address < instructionsBase) {
		return false;
	}
	// (pc-instructionsBase) is known to be non-negative at this time
	#pragma warning(disable:4018)
	if (address - instructionsBase >= instructions->size() * sizeof(int)) {
		return false;
	}
	return true;
}

int CPU::pcToMemoryOffset(int pc) {
	return pc * sizeof(int);
}

int CPU::pcToInstructionIndex(int pc) {
	return (pc * sizeof(int) - instructionsBase) / sizeof(int);
}
