#pragma once

#include "MemoryInterface.h"

/**
 * Interface to the previous memory level.
 * Takes a MemoryInterface and exposes only the part of the interface that is
 * relevant for the next level when serving the previous level.
 */
class PreviousMemoryLevel {
public:
	
	PreviousMemoryLevel(MemoryInterface* memoryInterface)
		: memoryInterface(memoryInterface) {}
	
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was read.
	 */
	bool getReadRequest(int* address, int notBefore);
	
	/** Responds to read request. */
	void respondRead(int address, int value, int notBefore);
	
	/**
	 * Reads the next write requested by the previous level if it's available.
	 * Returns whether address and value were read.
	 */
	bool getWriteRequest(int* address, int* value, int notBefore);

	/** Responds to write request. */
	void respondWrite(int address, int notBefore);

private:
	MemoryInterface* memoryInterface;
};
