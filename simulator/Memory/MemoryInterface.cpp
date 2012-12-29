#include "MemoryInterface.h"

void MemoryInterface::requestRead(int address, int now) {
	readRequests.push(address, now);
}

void MemoryInterface::respondRead(int address, int data, int now) {
	ReadResponse response;
	response.address = address;
	response.data = data;
	readResponses.push(response, now);
}

bool MemoryInterface::getReadRequest(int* address, int now) {
	return readRequests.pop(address, now);
}

bool MemoryInterface::getReadResponse(int* address, int& data, int now) {
	ReadResponse response;
	if (readResponses.pop(&response, now)) {
		*address = response.address;
		data = response.data;
		return true;
	}
	return false;
}

void MemoryInterface::requestWrite(int address, int data, int now) {
	WriteRequest request;
	request.address = address;
	request.data = data;
	writeRequests.push(request, now);
}

bool MemoryInterface::getWriteRequest(int* address, int& data, int now) {
	WriteRequest request;
	if (writeRequests.pop(&request, now)) {
		*address = request.address;
		data = request.data;
		return true;
	}
	return false;
}
