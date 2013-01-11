#include "MemoryInterface.h"

void MemoryInterface::requestRead(int address, int notBefore) {
	readRequests.push(address, notBefore);
}

void MemoryInterface::respondRead(int address, int value, int notBefore) {
	ReadResponse response;
	response.address = address;
	response.value = value;
	readResponses.push(response, notBefore);
}

bool MemoryInterface::getReadRequest(int* address, int notBefore) {
	return readRequests.pop(address, notBefore);
}

bool MemoryInterface::getReadResponse(int* address, int* value, int notBefore) {
	ReadResponse response;
	if (readResponses.pop(&response, notBefore)) {
		*address = response.address;
		*value = response.value;
		return true;
	}
	return false;
}

void MemoryInterface::requestWrite(int address, int value, int notBefore) {
	WriteRequest request;
	request.address = address;
	request.value = value;
	writeRequests.push(request, notBefore);
}

void MemoryInterface::respondWrite(int address, int notBefore) {
	writeResponses.push(address, notBefore);
}

bool MemoryInterface::getWriteRequest(int* address, int* value, int notBefore) {
	WriteRequest request;
	if (writeRequests.pop(&request, notBefore)) {
		*address = request.address;
		*value = request.value;
		return true;
	}
	return false;
}

bool MemoryInterface::getWriteResponse(int* address, int notBefore) {
	return writeResponses.pop(address, notBefore);
}
