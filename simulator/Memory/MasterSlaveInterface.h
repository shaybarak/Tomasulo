#pragma once

class MasterSlaveInterface {
public:
	int address;
	int data;
	bool slaveBusy;
	bool masterBusy;
	bool slaveReady;
	bool masterReady;
	bool slaveValid;
	bool masterValid;
	bool writeEnable;
};