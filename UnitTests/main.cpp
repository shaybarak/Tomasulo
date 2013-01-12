#include "../simulator/Memory/MainMemory.h"
#include "../simulator/Memory/L1Cache.h"
#include "../simulator/MIPS32/ISA.h"

#define FAILURE		0
#define SUCCESS		1

#define L1_BLOCK	8
#define L1_SIZE		128
#define L1_DELAY	1

#define L2_BLOCK	64
#define L2_SIZE		1024
#define L2_DELAY	10

#define RAM_DELAY	100

bool assertTrue(bool condition, int line, char* message) {
	if (!condition) {
		printf("%d: %s\n", line, message);
	}
	return condition;
}

int testRam() {
	int now = 0;
	MasterSlaveInterface l2RamInterface;
	MainMemory ram = MainMemory(RAM_DELAY, L2_BLOCK, &l2RamInterface);

	// Test correct initialization of signals
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should not be busy at startup")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == false, __LINE__, "RAM should not output valid signals at startup")) return FAILURE;

	// Test first read
	ram.pL2Master->address = 4;
	ram.pL2Master->masterReady = true;
	ram.pL2Master->writeEnable = false;
	ram.pL2Master->masterValid = true;
	for (int i = 1; i < RAM_DELAY; i++) {
		ram.onTickUp(++now);
		ram.onTickDown(now);
		if (!assertTrue(ram.pL2Master->slaveReady == false, __LINE__, "RAM should be busy")) return FAILURE;
		if (!assertTrue(ram.pL2Master->slaveValid == false, __LINE__, "RAM should not have output yet")) return FAILURE;
	}
	ram.onTickUp(++now);
	ram.onTickDown(now);
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should be done reading")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == true, __LINE__, "RAM should have valid outputs")) return FAILURE;
	if (!assertTrue(ram.pL2Master->address == 4, __LINE__, "RAM should maintain address signal")) return FAILURE;
	if (!assertTrue(ram.pL2Master->writeEnable == false, __LINE__, "RAM should maintain write enable signal")) return FAILURE;
	if (!assertTrue(ram.pL2Master->data == 0, __LINE__, "RAM should be initialized to zeroes")) return FAILURE;

	// Test subsequent read inside same row
	ram.pL2Master->address = 18;
	ram.pL2Master->writeEnable = false;
	ram.pL2Master->masterValid = true;
	ram.onTickUp(++now);
	ram.onTickDown(now);
	// Expect RAM to finish within 1 cycle since the row is already open
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should be done reading")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == true, __LINE__, "RAM should have valid outputs")) return FAILURE;

	// Test subsequent write
	ram.pL2Master->address = 18;
	ram.pL2Master->data = 17;
	ram.pL2Master->writeEnable = true;
	ram.pL2Master->masterValid = true;
	ram.onTickUp(++now);
	ram.onTickDown(now);
	// Expect RAM to finish within 1 cycle since the row is already open
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should be done writing")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == false, __LINE__, "RAM should have invalid outputs")) return FAILURE;

	// Test read of written value
	ram.pL2Master->address = 18;
	ram.pL2Master->data = 42;  // Alter data signal on purpose
	ram.pL2Master->writeEnable = false;
	ram.pL2Master->masterValid = true;
	ram.onTickUp(++now);
	ram.onTickDown(now);
	// Expect RAM to finish within 1 cycle since the row is already open
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should be done reading")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == true, __LINE__, "RAM should have valid outputs")) return FAILURE;
	if (!assertTrue(ram.pL2Master->data == 17, __LINE__, "RAM did not read correct written value")) return FAILURE;

	// Test read outside open row
	ram.pL2Master->address = 1000;
	ram.pL2Master->writeEnable = false;
	ram.pL2Master->masterValid = true;
	for (int i = 1; i < RAM_DELAY; i++) {
		ram.onTickUp(++now);
		ram.onTickDown(now);
		if (!assertTrue(ram.pL2Master->slaveReady == false, __LINE__, "RAM should be busy")) return FAILURE;
		if (!assertTrue(ram.pL2Master->slaveValid == false, __LINE__, "RAM should not have output yet")) return FAILURE;
	}
	ram.onTickUp(++now);
	ram.onTickDown(now);
	if (!assertTrue(ram.pL2Master->slaveReady == true, __LINE__, "RAM should be done reading")) return FAILURE;
	if (!assertTrue(ram.pL2Master->slaveValid == true, __LINE__, "RAM should have valid outputs")) return FAILURE;
	return SUCCESS;
}

int testL1Cache() {
	// TODO also test hit/miss count

	MasterSlaveInterface mockL2();
	L1Cache l1(L1_BLOCK, L1_SIZE, L1_DELAY, &mockL2);

	// Test correct initialization of signals
	if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should not be busy at startup") return FAILURE;
	if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should not output valid signals at startup") return FAILURE;

	// Test first read (expecting miss)
	l1.address = ISA::CODE_BASE + sizeof(int) * 2;
	l1.writeEnable = false;
	l1.masterValid = true;
	l1.onTickUp();
	l1.onTickDown();
	if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
	if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should not output valid signals during read miss") return FAILURE;
	l1.onTickUp();
	if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
	if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should not output valid signals during read miss") return FAILURE;
	if !assertTrue(mockL2.masterValid == true, __LINE__, "L1 cache should request read from L2") return FAILURE;
	if !assertTrue(mockL2.address == ISA::CODE_BASE + sizeof(int) * 2, __LINE__, "L1 cache should request read from L2") return FAILURE;
	if !assertTrue(mockL2.writeEnable == false, __LINE__, "L1 cache should request read from L2") return FAILURE;
	mockL2.slaveReady = false;  // Because we'll be streaming the rest of the block
	mockL2.slaveValid = false;
	
	// Take 17 cycles to serve request from L2
	for (int i = 0; i < 17; i++) {
		l1.onTickDown();
		if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
		if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should not output valid signals during read miss") return FAILURE;
		l1.onTickUp();
		if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
		if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should not output valid signals during read miss") return FAILURE;
	}
	mockL2.data = 42;
	mockL2.slaveValid = true;
	mockL2.slaveReady = false;
	l1.onTickDown();
	if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
	if !assertTrue(l1.slaveValid == true, __LINE__, "L1 cache should have valid output forwarded from L2 cache (critical word first)") return FAILURE;
	if !assertTrue(l1.address == ISA::CODE_BASE + sizeof(int) * 2, __LINE__, "L1 cache should maintain address on read response") return FAILURE;
	if !assertTrue(l1.data == 42, __LINE__, "L1 cache should forward correct value from L2 cache") return FAILURE;

	// Now CPU reads next instruction, expect hit since it is streamed from L2
	l1.address = ISA::CODE_BASE + sizeof(int) * 3;
	l1.writeEnable = false;
	l1.masterValid = true;
	l1.onTickUp();
	// L1 doesn't request read because L2 is not ready
	if !assertTrue(mockL2.masterValid == false, __LINE__, "L1 cache shouldn't read from L2 while L2 is busy") return FAILURE;
	mockL2.address = ISA::CODE_BASE + sizeof(int) * 3;
	mockL2.data = 17;
	mockL2.slaveValid = true;
	mockL2.slaveReady = false;
	l1.onTickDown();
	if !assertTrue(l1.slaveReady == true, __LINE__, "L1 cache should be ready for write") return FAILURE;
	if !assertTrue(l1.slaveValid == true, __LINE__, "L1 cache should have valid output forwarded from L2 cache") return FAILURE;
	if !assertTrue(l1.address == ISA::CODE_BASE + sizeof(int) * 3, __LINE__, "L1 cache should maintain address on read response") return FAILURE;
	if !assertTrue(l1.data == 17, __LINE__, "L1 cache should forward correct value from L2 cache") return FAILURE;
	
	// Now CPU reads next instruction which is not yet in cache but from the same block (stalling one extra cycle)
	l1.address = ISA::CODE_BASE + sizeof(int);
	l1.writeEnable = false;
	l1.masterValid = true;
	l1.onTickUp();
	if !assertTrue(mockL2.masterValid == true, __LINE__, "L1 cache shouldn't read from L2 while L2 is busy") return FAILURE;
	mockL2.address = ISA::CODE_BASE;
	mockL2.data = 18;
	mockL2.slaveValid = true;
	mockL2.slaveReady = false;
	l1.onTickDown();
	if !assertTrue(l1.slaveValid == false, __LINE__, "L1 cache should be stalled on L2 cache") return FAILURE;
	l1.onTickUp();
	mockL2.address = ISA::CODE_BASE + sizeof(int);
	mockL2.data = 19;
	mockL2.slaveValid = true;
	mockL2.slaveReady = true;  // Finished streaming L1 block from L2 cache
	l1.onTickDown();
	if !assertTrue(l1.slaveValid == true, __LINE__, "L1 cache should have valid output forwarded from L2 cache") return FAILURE;
	if !assertTrue(l1.address == ISA::CODE_BASE + sizeof(int), __LINE__, "L1 cache should maintain address on read response") return FAILURE;
	if !assertTrue(l1.data == 19, __LINE__, "L1 cache should forward correct value from L2 cache") return FAILURE;

	// Now CPU requests a cached instruction
	l1.address = ISA::CODE_BASE;
	l1.writeEnable = false;
	l1.masterValid = true;
	l1.onTickUp();
	l1.onTickDown();
	if !assertTrue(l1.slaveValid == true, __LINE__, "L1 cache should have valid output from its cache") return FAILURE;
	if !assertTrue(l1.address == ISA::CODE_BASE, __LINE__, "L1 cache should maintain address on read response") return FAILURE;
	if !assertTrue(l1.data == 18, __LINE__, "L1 cache should return correct value") return FAILURE;

	// Now write some data
	l1.address = 40;
	l1.data = 69;
	l1.writeEnable = true;
	l1.masterValid = true;
	l1.onTickUp();
	// TODO check write-allocate behavior
	// TODO check blocking on write buffer

	return SUCCESS;
}

int main() {
	return testRam() && testL1Cache();
}
