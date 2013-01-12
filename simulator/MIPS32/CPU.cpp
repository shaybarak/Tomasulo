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

void CPU::onTickUp(int now) {
	switch (state) {
	case READY:
		requestReadInstruction();
		memoryAccessCount++;
		state = INST_STALL;
		break;
	case INST_STALL:
		break;
	case LW:
		requestReadData(nextLwAddress);
		memoryAccessCount++;
		state = LW_STALL;
		break;
	case LW_STALL:
		break;
	case SW:
		if (pL1Slave->slaveReady) {
			requestWrite(nextSwAddress, nextSwData);
			memoryAccessCount++;
			state = SW_STALL;
		}
		break;
	case SW_STALL:
		break;
	case HALT:
		break;
	default:
		cerr << "CPU reached illegal state: " << (state) << "!" << endl;
		state = HALT;
		break; 
	}
}

void CPU::onTickDown(int now) {
	switch (state) {
	case READY:
		break;
	case INST_STALL:
		if (!pL1Slave->slaveValid) {
			break;
		}
		execute(pL1Slave->data);
		break;
	case LW:
		cerr << "CPU exception: illegal opcode " << (pL1Slave->data) << "!" << endl;
		state = HALT;
		break;
	case LW_STALL:
		if (!pL1Slave->slaveValid) {
			break;
		}
		continueExecuteLw();
		state = READY;
		break;
	case SW:
		break;
	case SW_STALL:
		if (pL1Slave->slaveReady) {
			state = READY;
		}
		continueExecuteSw();
		break;
	case HALT:
		break;
	default:
		cerr << "CPU reached illegal state: " << (state) << "!" << endl;
		state = HALT;
		break; 
	}
}

//TODO: change to general method on MasterSlaveInterface "MasterRequestRead/Write"
void CPU::requestReadInstruction() {
	pL1Slave->writeEnable = false;
	pL1Slave->masterValid = false;
	pL1Slave->masterReady = true;
	pL1Slave->address = pcToMemoryOffset(pc);
}

void CPU::requestReadData(int address) {
	pL1Slave->writeEnable = false;
	pL1Slave->masterValid = false;
	pL1Slave->masterReady = true;
	pL1Slave->address = address;
}

void CPU::requestWrite(int address, int data) {
	pL1Slave->writeEnable = true;
	pL1Slave->masterValid = false;
	pL1Slave->masterReady = true;
	pL1Slave->address = address;
	pL1Slave->data = data;
}


void CPU::execute(int instructionIndex) {
	if (pL1Slave->data != pcToInstructionIndex(pc)) {
		cerr << "CPU exception: illegal opcode " << (pL1Slave->data) << "!" << endl;
		state = HALT;
		return;
	}
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	Instruction* instruction = instructions->at(instructionIndex);
	int data;
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
		nextLwAddress = (*gpr)[itype->getRs()] + itype->getImmediate();
		nextLwRt = (*gpr)[itype->getRt()];
		if (!isValidMemoryAddress(nextLwAddress)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			state = HALT;
			break;
		}
		state = LW;
		memoryAccessCount++;
		// Note: stalled, don't advance PC and don't count instruction as committed
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		nextSwAddress = (*gpr)[itype->getRs()] + itype->getImmediate();
		nextSwData = (*gpr)[itype->getRt()];
		if (!isValidMemoryAddress(nextSwAddress)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			halted = true;
			break;
		}
		memoryAccessCount++;
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
		state = HALT;
		instructionsCommitted++;
		break;
	default:
		cerr << "CPU exception: invalid opcode!" << endl;
		state = HALT;
		break;
	}
}

void CPU::continueExecuteLw() {
	(*gpr)[nextLwRt] = pL1Slave->data;
}

void CPU::continueExecuteSw() {
	pL1Slave->address = nextSwAddress;
	pL1Slave->data = nextSwData;
	pL1Slave->masterValid = nextSwData;
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
