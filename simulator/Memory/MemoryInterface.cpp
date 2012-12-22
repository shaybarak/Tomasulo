#include "MemoryInterface.h"

template <typename AddressType, typename DataType>
void MemoryInterface::requestRead(AddressType address, int now) {
	readRequests.push(address, now);
}

template <typename AddressType, typename DataType>
void MemoryInterface::respondRead(AddressType address, DataType data, int now) {
	ReadResponse response;
	response.address = address;
	response.data = data;
	readResponses.push(response, now);
}

template <typename AddressType, typename DataType>
bool MemoryInterface::getReadRequest(AddressType* address, int now) {
	return readRequests.pop(address, now);
}

template <typename AddressType, typename DataType>
bool MemoryInterface::getReadResponse(AddressType* address, DataType* data, int now) {
	ReadResponse response;
	if (readResponses.pop(&response, now)) {
		*address = response.address;
		*data = response.data;
		return true;
	}
	return false;
}

template <typename AddressType, typename DataType>
void MemoryInterface::requestWrite(AddressType address, DataType data, int now) {
	WriteRequest request;
	request.address = address;
	request.data = data;
	writeRequests.push(request, now);
}

template <typename AddressType, typename DataType>
bool MemoryInterface::getWriteRequest(AddressType* address, DataType* data, int now) {
	WriteRequest request;
	if (writeRequests.pop(&request, now)) {
		*address = request.address;
		*data = request.data;
		return true;
	}
	return false;
}
