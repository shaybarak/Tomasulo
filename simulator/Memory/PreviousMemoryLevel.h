#pragma once

#include "MemoryInterface.h"

/**
 * Interface to the previous memory level.
 * Takes a MemoryInterface and exposes only the part of the interface that is
 * relevant for the next level when serving the previous level.
 */
template <typename AddressType = int, typename DataType = int>
class PreviousMemoryLevel {
public:
	PreviousMemoryLevel(MemoryInterface<AddressType, DataType>* memoryInterface)
		: memoryInterface(memoryInterface) {}
	/** Responds to data read request. */
	void respondRead(AddressType address, DataType data, int now);
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was read.
	 */
	bool getReadRequest(AddressType* address, int now);
	bool getWriteRequest(AddressType* address, DataType* data, int now);
	// There are no write responses. Writer doesn't wait for confirmation of commit.

private:
	MemoryInterface<AddressType, DataType>* memoryInterface;
};