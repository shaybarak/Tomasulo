#include "CPU.h"
#include "Instruction.h"
#include "RTypeInstruction.h"
#include "ITypeInstruction.h"
#include "JTypeInstruction.h"

void CPU::loadProgram(vector<Instruction*>* instructions, int pc) {
	this->instructions = instructions;
	this->pc = pc;
	state = READY;
}

void CPU::onTickUp(int now) {
	switch (state) {
	case READY:
		requestReadInst();
		state = INST_STALL;
		break;
	case INST_STALL:
		break;
	case LW:
		if (pL1DataSlave->slaveReady) {
			requestReadData(nextLwAddress);
			state = LW_STALL;
		}
		break;
	case LW_STALL:
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
		if (!pL1InstSlave->slaveValid) {
			break;
		}
		execute(pL1InstSlave->data);
		break;
	case LW:
		if (!pL1DataSlave->slaveReady) {
			cerr << "CPU exception: cannot be in LW state on tick down!" << endl;
			state = HALT;
		}
		break;
	case LW_STALL:
		if (!pL1DataSlave->slaveValid) {
			break;
		}
		continueExecuteLw();
		state = READY;
		break;
	case SW_STALL:
		if (pL1DataSlave->slaveReady) {
			requestWrite(nextSwAddress, nextSwData);
			continueExecuteSw();
			state = READY;
		}		
		break;
	case HALT:
		break;
	default:
		cerr << "CPU reached illegal state: " << (state) << "!" << endl;
		state = HALT;
		break; 
	}
}

void CPU::execute(int instructionIndex) {
	if (pL1InstSlave->data != pc) {
		cerr << "CPU exception: illegal opcode " << (pL1InstSlave->data) << "!" << endl;
		state = HALT;
		return;
	}
	state = READY;
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;
	Instruction* instruction = instructions->at(instructionIndex);
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
		// Note: stalled, don't advance PC and don't count instruction as committed
		break;
	case ISA::sw:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		nextSwAddress = (*gpr)[itype->getRs()] + itype->getImmediate();
		nextSwData = (*gpr)[itype->getRt()];
		if (!isValidMemoryAddress(nextSwAddress)) {
			cerr << "CPU exception: memory offset out of range!" << endl;
			state = HALT;
			break;
		}
		if (pL1DataSlave->slaveReady) {
			//write buffer enables trying to write on same cycle without waiting access time.
			requestWrite(nextSwAddress, nextSwData);
			continueExecuteSw();
			state = READY;
			break;
		}
		state = SW_STALL;
		break;
	case ISA::beq:
		itype = dynamic_cast<ITypeInstruction*>(instruction);
		if ((*gpr)[itype->getRs()] == (*gpr)[itype->getRt()]) {
			pc += itype->getImmediate();
		}
		pc++;
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

//TODO: change to general method on MasterSlaveInterface "MasterRequestRead/Write"
void CPU::requestReadInst() {
	pL1InstSlave->writeEnable = false;
	pL1InstSlave->masterValid = true;
	pL1InstSlave->masterReady = true;
	pL1InstSlave->address = pcToMemoryOffset(pc);
	memoryAccessCount++;
}

void CPU::requestReadData(int address) {
	pL1DataSlave->writeEnable = false;
	pL1DataSlave->masterValid = false;
	pL1DataSlave->masterReady = true;
	pL1DataSlave->address = address;
	memoryAccessCount++;
}

void CPU::requestWrite(int address, int data) {
	pL1DataSlave->writeEnable = true;
	pL1DataSlave->masterValid = true;
	pL1DataSlave->masterReady = true;
	pL1DataSlave->address = address;
	pL1DataSlave->data = data;
	memoryAccessCount++;
}

void CPU::continueExecuteLw() {
	(*gpr)[nextLwRt] = pL1DataSlave->data;
	pc++;
	instructionsCommitted++;
}

void CPU::continueExecuteSw() {
	pc++;
	instructionsCommitted++;
}

bool CPU::isValidMemoryAddress(int address) {
	return (address >= 0) && (address < memorySize);
}

bool CPU::isValidInstructionAddress(int address) {
	return ((address >= 0) && (address < (int)(instructions->size() * sizeof(int))));
}

int CPU::pcToMemoryOffset(int pc) {
	return pc * sizeof(int);
}
