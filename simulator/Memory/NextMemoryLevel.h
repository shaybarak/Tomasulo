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
	/** Requests to read data from the next memory level. */
	void requestRead(int address, int now);
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(int* address, int& data, int now);
	/** Registers a write request to the next memory level. */
	void requestWrite(int address, int data, int now);

private:
	MemoryInterface* memoryInterface;
};
