#include "NextMemoryLevel.h"

void NextMemoryLevel::requestRead(int address, int notBefore);
	memoryInterface->responsdRead(address, data, now);
}

bool NextMemoryLevel::getReadResponse(int* address, int& data, int now) {
	return memoryInterface->getReadResponse(address, data, now);
}

void NextMemoryLevel::requestWrite(int address, int data, int notBefore) {
	memoryInterface->requestWrite(address, data, now);
}
