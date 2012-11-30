#include "Labeler.h"
#include "ISA.h"

const regex LabelAnalyzer::labeledInstruction("^\\s*(\\w*)\\s*:\\s*\\w*\\s*.*$");
const regex LabelAnalyzer::unlabeledInstruction("^\\s*\\w*\\s*.*$");

unsigned int LabelAnalyzer::parse(string& line) {
	smatch match;
	if (regex_search(line, match, labeledInstruction)) {
		labels[match[0].str()] = pc;
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

const map<string, ISA::Address>& LabelAnalyzer::getLabels() const {
	return labels;
}