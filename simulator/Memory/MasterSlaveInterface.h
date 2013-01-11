#pragma once

class MasterSlaveInterface {
public:
	int address;
	int data;
	bool slaveReady;
	bool masterReady;
	bool slaveValid;
	bool masterValid;
	bool writeEnable;

	//writeEnable + slaveReady => write was succefully committed
};