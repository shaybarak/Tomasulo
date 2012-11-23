#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

// Instruction abstract base class
class Instruction {
public:
	Instruction(string& opcode) : opcode(opcode) {}
	virtual ~Instruction() = 0;
	const string& getOpcode();

private:
	string opcode;
};