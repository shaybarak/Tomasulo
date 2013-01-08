#pragma once

#include "MemoryInterface.h"

/**
 * Interface to the next memory level.
 * Takes a MemoryInterface and exposes only the part of the interface that is
 * relevant for the previous level when using the next level.
 */
class NextMemoryLevel {
public:

	NextMemoryLevel(MemoryInterface* memoryInterface)
		: memoryInterface(memoryInterface) {}
	
	/** Requests to read from the next memory level. */
	void requestRead(int address, int notBefore);
	
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(int* address, int* value, int notBefore);
	
	/** Registers a write request to the next memory level. */
	void requestWrite(int address, int value, int notBefore);
	
	/** Reads acknowledgement for write request. */
	bool getWriteResponse(int* address, int notBefore);

private:
	MemoryInterface* memoryInterface;
};
