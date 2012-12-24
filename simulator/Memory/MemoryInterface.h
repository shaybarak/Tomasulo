#pragma once

#include "TimedQueue.h"

/**
 * Base class for an interface between any two memory levels in any particular direction.
 * E.g. CPU->L1 or L1<-CPU etc'.
 */
class MemoryInterface {
public:
	/** Requests to read data from the next memory level. */
	void requestRead(int address, int now);
	/** Responds to data read request. */
	void respondRead(int address, int data, int now);
	/**
	 * Reads the next address requested by the previous level if it's available.
	 * Returns whether address was read.
	 */
	bool getReadRequest(int* address, int now);
	/**
	 * Reads the next value read from the next memory level if it's available.
	 * Returns whether value was read.
	 */
	bool getReadResponse(int* address, int& data, int now);
	/** Registers a write request to the next memory level. */
	void requestWrite(int address, int data, int now);
	/**
	 * Reads the next write requested by the previous level if it's available.
	 * Returns whether request was read.
	 */
	bool getWriteRequest(int* address, int& data, int now);
	// There are no write responses. Writer doesn't wait for confirmation of commit.

private:
	typedef int ReadRequest;
	typedef struct AddressData {
		int address;
		int data;
	} ReadResponse, WriteRequest;
	TimedQueue<ReadRequest> readRequests;
	TimedQueue<ReadResponse> readResponses;
	TimedQueue<WriteRequest> writeRequests;
};
