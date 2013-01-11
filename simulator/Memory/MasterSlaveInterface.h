#pragma once

class MasterSlaveInterface {
public:
	
	MasterSlaveInterface(): slaveReady(true), slaveValid(false), masterReady(false), 
			masterValid(false), writeEnable(false){}

	int address;
	int data;
	bool slaveReady;
	bool slaveValid;
	bool masterReady;
	bool masterValid;
	bool writeEnable;
	//writeEnable + slaveReady => write was succefully committed
};