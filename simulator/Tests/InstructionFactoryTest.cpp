#include "../MIPS32/InstructionFactory.h"
#include "../MIPS32/Instruction.h"
#include "../MIPS32/RTypeInstruction.h"
#include "../MIPS32/ITypeInstruction.h"
#include "../MIPS32/JTypeInstruction.h"
#include "../MIPS32/Labeler.h"
#include <string>
#include <map>

int main() {
	// TODO test instructions at different addresses

	Labeler::Labels labels;
	labels["L1"] = 100;
	labels["sheker"] = 107;
	InstructionFactory factory(labels, 100);
	Instruction* instruction = NULL;
	RTypeInstruction* rtype = NULL;
	ITypeInstruction* itype = NULL;
	JTypeInstruction* jtype = NULL;

	// Test the first example instructions from the exercise documentation
	instruction = factory.parse(string("L1: ADD $1 $2 $3"));
	if (instruction->getOpcode() != ISA::add) {
		cerr << "Test 1 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 10;
	}
	rtype = dynamic_cast<RTypeInstruction*>(instruction);
	if (rtype->getRs() != 1) {
		cerr << "Test 1 failed: wrong rs " << rtype->getRs() << endl;
		return 12;
	}
	if (rtype->getRt() != 2) {
		cerr << "Test 1 failed: wrong rt " << rtype->getRt() << endl;
		return 13;
	}
	if (rtype->getRd() != 3) {
		cerr << "Test 1 failed: wrong rd " << rtype->getRd() << endl;
		return 14;
	}

	// Test the second example instructions from the exercise documentation
	instruction = factory.parse(string("MUL $2 $1 $4"));
	if (instruction->getOpcode() != ISA::mul) {
		cerr << "Test 2 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 20;
	}
	rtype = dynamic_cast<RTypeInstruction*>(instruction);
	if (rtype->getRs() != 2) {
		cerr << "Test 2 failed: wrong rs " << rtype->getRs() << endl;
		return 21;
	}
	if (rtype->getRt() != 1) {
		cerr << "Test 2 failed: wrong rt " << rtype->getRt() << endl;
		return 22;
	}
	if (rtype->getRd() != 4) {
		cerr << "Test 2 failed: wrong rd " << rtype->getRd() << endl;
		return 23;
	}

	// Another example
	instruction = factory.parse(string("subi $7 $8 9"));
	if (instruction->getOpcode() != ISA::subi) {
		cerr << "Test 3 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 30;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 7) {
		cerr << "Test 3 failed: wrong rs " << itype->getRs() << endl;
		return 31;
	}
	if (itype->getRt() != 8) {
		cerr << "Test 3 failed: wrong rt " << itype->getRt() << endl;
		return 32;
	}
	if (itype->getImmediate() != 9) {
		cerr << "Test 3 failed: wrong immediate " << itype->getImmediate() << endl;
		return 33;
	}

	// Another example
	instruction = factory.parse(string("sw $5,(-16)$3"));
	if (instruction->getOpcode() != ISA::sw) {
		cerr << "Test 4 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 40;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 5) {
		cerr << "Test 4 failed: wrong rs " << itype->getRs() << endl;
		return 41;
	}
	if (itype->getRt() != 3) {
		cerr << "Test 4 failed: wrong rt " << itype->getRt() << endl;
		return 42;
	}
	if (itype->getImmediate() != -16) {
		cerr << "Test 4 failed: wrong immediate " << itype->getImmediate() << endl;
		return 43;
	}

	// Another example
	instruction = factory.parse(string("j L1"));
	if (instruction->getOpcode() != ISA::j) {
		cerr << "Test 5 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 50;
	}
	jtype = dynamic_cast<JTypeInstruction*>(instruction);
	if (jtype->getTarget() != 100) {
		cerr << "Test 5 failed: wrong target " << jtype->getTarget() << endl;
		return 51;
	}

	// Another example
	instruction = factory.parse(string("halt"));
	if (instruction->getOpcode() != ISA::halt) {
		cerr << "Test 6 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 60;
	}

	// Unknown instruction
	instruction = factory.parse(string("blah: blah"));
	if (instruction != NULL) {
		cerr << "Test 7 failed: expecting invalid instruction" << endl;
		return 70;
	}

	// Register index out of range
	instruction = factory.parse(string("add $30 $31 $32"));
	if (instruction != NULL) {
		cerr << "Test 8 failed: expecting invalid instruction" << endl;
		return 80;
	}

	/////////////////////////////////////////////
	// Lines with comments
	/////////////////////////////////////////////

	//Only a comment
	instruction = factory.parse(string("// This is a comment"));
	if (instruction != NULL) {
		cerr << "Test 9 failed: expecting comment" << endl;
		return 90;
	}

	//Instruction with a comment
	instruction = factory.parse(string("sw $5,(-16)$3 //KAKI! This is a comment"));
	if (instruction->getOpcode() != ISA::sw) {
		cerr << "Test 4 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 91;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 5) {
		cerr << "Test 9 failed: wrong rs " << itype->getRs() << endl;
		return 92;
	}
	if (itype->getRt() != 3) {
		cerr << "Test 9 failed: wrong rt " << itype->getRt() << endl;
		return 93;
	}
	if (itype->getImmediate() != -16) {
		cerr << "Test 9 failed: wrong immediate " << itype->getImmediate() << endl;
		return 94;
	}

	// Only blanks
	instruction = factory.parse(string("  "));
	if (instruction != NULL) {
		cerr << "Test 10 failed: expecting blank line" << endl;
		return 100;
	}

	// Empty line
	instruction = factory.parse(string(""));
	if (instruction != NULL) {
		cerr << "Test 11 failed: expecting empty line" << endl;
		return 110;
	}

	// Relative branch
	instruction = factory.parse(string("bne $1 $2 L1"));
	if (instruction->getOpcode() != ISA::bne) {
		cerr << "Test 12 failed: wrong opcode " << instruction->getOpcode() << endl;
		return 120;
	}
	itype = dynamic_cast<ITypeInstruction*>(instruction);
	if (itype->getRs() != 1) {
		cerr << "Test 12 failed: wrong rs " << itype->getRs() << endl;
		return 92;
	}
	if (itype->getRt() != 2) {
		cerr << "Test 12 failed: wrong rt " << itype->getRt() << endl;
		return 93;
	}
	if (itype->getImmediate() != -32) {
		cerr << "Test 12 failed: wrong immediate " << itype->getImmediate() << endl;
		return 94;
	}
}