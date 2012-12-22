#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"

void CPU::loadProgram(vector<Instruction*>* instructions, int instructionsBase, int pc) {
	this.instructions = instructions;
	this.instructionsBase = instructionsBase;
	this.pc = pc;
	halted = false;
}

void onTick(int now) {
	this.now = now;
	// Don't execute if halted
	if (halted) {
		return;
	}

	// If stalled on reading instruction
	if (instructionReadStall) {
		int instruction;
		if (!l1CacheReadQueue->(&instruction, now)) {
			// Read did not return yet
			return;
		}
		// Verify that instruction was read correctly
		if (instruction != pc - instructionsBase) {
			cerr << "CPU exception: illegal opcode " << instruction << "!" << endl;
			halted = true;
			return;
		}
		instructionReadStall = false;
		execute(instruction);
		return;
	}

	// Otherwise if stalled on reading/writing data
	if (dataReadStall) {
		int data;
		if (!l1CacheReadQueue->(&data, now)) {
			// Read did not return yet
			return;
		}
		dataStall = false;
		execute(pc - instructionsBase, data);
		return;
	}

	// Otherwise read next instruction
	if (!isValidInstructionAddress(pc)) {
		cerr << "CPU exception: program counter out of range! PC=" << pc << endl;
		halted = true;
		return;
	}
	l1CacheQueue->push(pc, now);
	instructionReadStall = true;
}

int CPU::getInstructionsCommitted() const {
	return instructionsCommitted;
}

bool CPU::isHalted() const {
	return halted;
}

void execute(int instructionIndex) {
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	Instruction* instruction = instructions->at(pc - instructionsBase);
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
			pc--;
			instructionsCommitted--;
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
		int address = (*gpr)[itype->getRs()] + itype->getImmediate();
		if (!isValidAddress(address)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			halted = true;
			break;
		}
		l1CacheReadQueue->push(address, now);
		// Note: stalled, don't advance PC and don't count instruction as committed
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		int address = (*gpr)[itype->getRs()] + itype->getImmediate();
		int data = (*gpr)[itype->getRt()];
		if (!isValidAddress(address)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			halted = true;
			break;
		}
		WriteRequest request;
		request.address = address;
		request.data = data;
		l1CacheWriteQueue->push(request, now);
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

void execute(int instructionIndex, int data) {
	ITypeInstruction* itype = NULL;
	Instruction* instruction = instructions->at(pc - instructionsBase);
	pc++;
	instructionsCommitted++;
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
	if (address - instructionsBase >= instructions.size()) {
		return false;
	}
	return true;
}
