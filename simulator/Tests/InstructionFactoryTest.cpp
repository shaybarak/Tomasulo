#include "../MIPS32/InstructionFactory.h"
#include "../MIPS32/Instruction.h"
#include "../MIPS32/RTypeInstruction.h"
#include "../MIPS32/ITypeInstruction.h"
#include "../MIPS32/JTypeInstruction.h"
#include <string>
#include <map>

int main() {
	map<string, int> symbols;
	symbols["L1"] = 6;
	symbols["sheker"] = 7;
	InstructionFactory factory(symbols);
	Instruction* instruction = NULL;
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;

	// Test the first example instructions from the exercise documentation
	instruction = factory.parse(string("L1: ADD $1 $2 $3"));
	if (instruction->getOpcode() != Instruction::add) {
		cerr << "Test 1 failed: wrong opcode " << instruction->getOpcode() << endl;
	}
	rtype = dynamic_cast<RTypeInstruction*>(instruction);
	if (rtype->getRs() != 1) {
		cerr << "Test 1 failed: wrong rs " << rtype->getRs() << endl;
	}
	if (rtype->getRt() != 2) {
		cerr << "Test 1 failed: wrong rt " << rtype->getRt() << endl;
	}
	if (rtype->getRd() != 3) {
		cerr << "Test 1 failed: wrong rd " << rtype->getRd() << endl;
	}

	// Test the second example instructions from the exercise documentation
	instruction = factory.parse(string("MUL $2 $1 $4"));
	if (instruction->getOpcode() != Instruction::mul) {
		cerr << "Test 2 failed: wrong opcode " << instruction->getOpcode() << endl;
	}
	rtype = dynamic_cast<RTypeInstruction*>(instruction);
	if (rtype->getRs() != 2) {
		cerr << "Test 2 failed: wrong rs " << rtype->getRs() << endl;
	}
	if (rtype->getRt() != 1) {
		cerr << "Test 2 failed: wrong rt " << rtype->getRt() << endl;
	}
	if (rtype->getRd() != 4) {
		cerr << "Test 2 failed: wrong rd " << rtype->getRd() << endl;
	}

	// Another example
	instruction = factory.parse(string("subi $7 $8 9"));
	if (instruction->getOpcode() != Instruction::subi) {
		cerr << "Test 3 failed: wrong opcode " << instruction->getOpcode() << endl;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 7) {
		cerr << "Test 3 failed: wrong rs " << itype->getRs() << endl;
	}
	if (itype->getRt() != 8) {
		cerr << "Test 3 failed: wrong rt " << itype->getRt() << endl;
	}
	if (itype->getImmediate() != 9) {
		cerr << "Test 3 failed: wrong immediate " << itype->getImmediate() << endl;
	}

	// Another example
	instruction = factory.parse(string("sw $5,(-16)$3"));
	if (instruction->getOpcode() != Instruction::sw) {
		cerr << "Test 4 failed: wrong opcode " << instruction->getOpcode() << endl;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 5) {
		cerr << "Test 4 failed: wrong rs " << itype->getRs() << endl;
	}
	if (itype->getRt() != 3) {
		cerr << "Test 4 failed: wrong rt " << itype->getRt() << endl;
	}
	if (itype->getImmediate() != -16) {
		cerr << "Test 4 failed: wrong immediate " << itype->getImmediate() << endl;
	}

	// Another example
	instruction = factory.parse(string("j L1"));
	if (instruction->getOpcode() != Instruction::j) {
		cerr << "Test 5 failed: wrong opcode " << instruction->getOpcode() << endl;
	}
	jtype = dynamic_cast<JTypeInstruction*>(instruction);
	if (jtype->getTarget() != 6) {
		cerr << "Test 5 failed: wrong target " << jtype->getTarget() << endl;
	}

	// Another example
	instruction = factory.parse(string("halt"));
	if (instruction->getOpcode() != Instruction::halt) {
		cerr << "Test 6 failed: wrong opcode " << instruction->getOpcode() << endl;
	}

	// Unknown instruction
	instruction = factory.parse(string("blah: blah"));
	if (instruction != NULL) {
		cerr << "Test 7 failed: expecting invalid instruction" << endl;
	}

	// Register index out of range
	instruction = factory.parse(string("add $30 $31 $32"));
	if (instruction != NULL) {
		cerr << "Test 8 failed: expecting invalid instruction" << endl;
	}
}