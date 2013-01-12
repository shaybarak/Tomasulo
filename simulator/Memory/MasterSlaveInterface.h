#pragma once

class MasterSlaveInterface {
public:
	MasterSlaveInterface() : slaveReady(true), slaveValid(false), 
		masterValid(false), writeEnable(false) {}

	int address;
	int data;
	bool slaveReady;
	bool slaveValid;
	bool masterValid;
	bool writeEnable;
};
