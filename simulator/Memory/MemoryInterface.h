#pragma once

#include "TimedQueue.h"

/**
 * Base class for an interface between any two memory levels in any particular direction.
 * E.g. CPU->L1 or L1<-CPU etc'.
 */
template <typename AddressType = int, typename DataType = int>
class MemoryInterface {
public:
	/** Requests to read data from the next memory level. */
	void requestRead(AddressType address, int now);
	/** Responds to data read request. */
	void respondRead(AddressType address, DataType data, int now);
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was read.
	 */
	bool getReadRequest(AddressType* address, int now);
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(AddressType* address, DataType& data, int now);
	/** Registers a write request to the next memory level. */
	void requestWrite(AddressType address, DataType data, int now);
	/**
	 * Reads the next write requested by the previous level if it's available.
	 * Returns whether request was read.
	 */
	bool getWriteRequest(AddressType* address, DataType& data, int now);
	// There are no write responses. Writer doesn't wait for confirmation of commit.

private:
	typedef AddressType ReadRequest;
	typedef struct AddressData {
		AddressType address;
		DataType data;
	} ReadResponse, WriteRequest;
	TimedQueue<ReadRequest> readRequests;
	TimedQueue<ReadResponse> readResponses;
	TimedQueue<WriteRequest> writeRequests;
};
