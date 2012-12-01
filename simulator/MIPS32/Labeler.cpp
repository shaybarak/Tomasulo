#include "Labeler.h"
#include "ISA.h"

const regex Labeler::labeledInstruction("^\\s*(\\w*)\\s*:\\s*\\w*\\s*.*$");
const regex Labeler::unlabeledInstruction("^\\s*\\w*\\s*.*$");

unsigned int Labeler::parse(string& line) {
	smatch match;
	if (regex_search(line, match, labeledInstruction)) {
		labels[match[1].str()] = pc;
		// Advance to next instruction
		pc++;
	} else if (regex_match(line, unlabeledInstruction)) {
		// Real instruction, just unlabeled
		pc++;
	} else {
		// Comment or empty line or whatever, ignore
	}
	return pc;
}

const map<string, int>& Labeler::getLabels() const {
	return labels;
}