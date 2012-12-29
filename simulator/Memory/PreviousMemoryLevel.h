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
	/** Responds to data read request. */
	void respondRead(int address, int data, int notBefore);
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was read.
	 */
	bool getReadRequest(int* address, int notBefore);
	bool getWriteRequest(int* address, int* data, int notBefore);
	// There are no write responses. Writer doesn't wait for confirmation of commit.

private:
	MemoryInterface* memoryInterface;
};
