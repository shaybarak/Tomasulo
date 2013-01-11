#pragma once

#include "TimedQueue.h"

/**
 * Base class for an interface between any two memory levels in any particular direction.
 * E.g. CPU->L1 or L1<-CPU etc'.
 */
class MemoryInterface {
public:
	
	/** Requests to read from the next memory level. */
	void requestRead(int address, int notBefore);
	
	/** Responds to read request. */
	void respondRead(int address, int value, int notBefore);
	
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was received.
	 */
	bool getReadRequest(int* address, int notBefore);
	
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(int* address, int* value, int notBefore);
	
	/** Registers a write request to the next memory level. */
	void requestWrite(int address, int value, int notBefore);
	
	/** Responds to a write request. */
	void respondWrite(int address, int notBefore);
	
	/**
	 * Reads the next write requested by the previous level if it's available.
	 * Returns whether request was received.
	 */
	bool getWriteRequest(int* address, int* value, int notBefore);

	/**
	 * Reads the next write confirmation by the next level if it's available.
	 * Returns whether confirmation was received.
	 */
	bool getWriteResponse(int* address, int notBefore);

private:
	typedef int ReadRequest, WriteResponse;
	typedef struct AddressData {
		int address;
		int value;
	} ReadResponse, WriteRequest;
	TimedQueue<ReadRequest> readRequests;
	TimedQueue<ReadResponse> readResponses;
	TimedQueue<WriteRequest> writeRequests;
	TimedQueue<WriteResponse> writeResponses;
};
