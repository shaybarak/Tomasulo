#pragma once

#include "MemoryInterface.h"

/**
 * Interface to the next memory level.
 * Takes a MemoryInterface and exposes only the part of the interface that is
 * relevant for the previous level when using the next level.
 */
template <typename AddressType = int, typename DataType = int>
class NextMemoryLevel {
public:
	NextMemoryLevel(MemoryInterface<AddressType, DataType>* memoryInterface)
		: memoryInterface(memoryInterface) {}
	/** Requests to read data from the next memory level. */
	void requestRead(AddressType address, int now);
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(AddressType* address, DataType& data, int now);
	/** Registers a write request to the next memory level. */
	void requestWrite(AddressType address, DataType data, int now);

private:
	MemoryInterface<AddressType, DataType>* memoryInterface;
};
