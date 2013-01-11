#include "../simulator/Memory/MainMemory.h"

#define FAILURE		0
#define SUCCESS		1

#define L1_DELAY	1
#define L2_DELAY	10
#define RAM_DELAY	100

#define L1_BLOCK	8
#define L2_BLOCK	64

bool assertTrue(bool condition, int line, char* message) {
	if (!condition) {
		printf("%d: %s\n", line, message);
	}
	return condition;
}

int testRam() {
	MainMemory ram = new MainMemory(RAM_DELAY, L2_BLOCK);

	// Test correct initialization of signals
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should not be busy at startup") return FAILURE;
	if !assertTrue(ram.slaveValid == false, __LINE__, "RAM should not output valid signals at startup") return FAILURE;

	// Test first read
	ram.address = 4;
	ram.writeEnable = false;
	ram.masterValid = true;
	for (int i = 1; i < RAM_DELAY; i++) {
		ram.onTickUp();
		ram.onTickDown();
		if !assertTrue(ram.slaveReady == false, __LINE__, "RAM should be busy") return FAILURE;
		if !assertTrue(ram.slaveValid == false, __LINE__, "RAM should not have output yet") return FAILURE;
	}
	ram.onTickUp();
	ram.onTickDown();
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should be done reading") return FAILURE;
	if !assertTrue(ram.slaveValid == true, __LINE__, "RAM should have valid outputs") return FAILURE;
	if !assertTrue(ram.address == 4, __LINE__, "RAM should maintain address signal") return FAILURE;
	if !assertTrue(ram.writeEnable == false, __LINE__, "RAM should maintain write enable signal") return FAILURE;
	if !assertTrue(ram.data == 0, __LINE__, "RAM should be initialized to zeroes") return FAILURE;

	// Test subsequent read inside same row
	ram.address = 18;
	ram.writeEnable = false;
	ram.masterValid = true;
	ram.onTickUp();
	ram.onTickDown();
	// Expect RAM to finish within 1 cycle since the row is already open
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should be done reading") return FAILURE;
	if !assertTrue(ram.slaveValid == true, __LINE__, "RAM should have valid outputs") return FAILURE;

	// Test subsequent write
	ram.address = 18;
	ram.data = 17;
	ram.writeEnable = true;
	ram.masterValid = true;
	ram.onTickUp();
	ram.onTickDown();
	// Expect RAM to finish within 1 cycle since the row is already open
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should be done reading") return FAILURE;
	if !assertTrue(ram.slaveValid == true, __LINE__, "RAM should have valid outputs") return FAILURE;

	// Test read of written value
	ram.address = 18;
	ram.data = 42;  // Alter data signal on purpose
	ram.writeEnable = false;
	ram.masterValid = true;
	ram.onTickUp();
	ram.onTickDown();
	// Expect RAM to finish within 1 cycle since the row is already open
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should be done reading") return FAILURE;
	if !assertTrue(ram.slaveValid == true, __LINE__, "RAM should have valid outputs") return FAILURE;
	if !assertTrue(ram.data == 17, __LINE__, "RAM did not read correct written value") return FAILURE;

	// Test read outside open row
	ram.address = 1000;
	ram.writeEnable = false;
	ram.masterValid = true;
	for (int i = 1; i < RAM_DELAY; i++) {
		ram.onTickUp();
		ram.onTickDown();
		if !assertTrue(ram.slaveReady == false, __LINE__, "RAM should be busy") return FAILURE;
		if !assertTrue(ram.slaveValid == false, __LINE__, "RAM should not have output yet") return FAILURE;
	}
	ram.onTickUp();
	ram.onTickDown();
	if !assertTrue(ram.slaveReady == true, __LINE__, "RAM should be done reading") return FAILURE;
	if !assertTrue(ram.slaveValid == true, __LINE__, "RAM should have valid outputs") return FAILURE;
}

int main() {
	return testRam();
}
