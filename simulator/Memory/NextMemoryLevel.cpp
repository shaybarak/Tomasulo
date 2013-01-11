#include "NextMemoryLevel.h"

void NextMemoryLevel::requestRead(int address, int notBefore) {
	memoryInterface->requestRead(address, notBefore);
}

bool NextMemoryLevel::getReadResponse(int* address, int* value, int notBefore) {
	return memoryInterface->getReadResponse(address, value, notBefore);
}

void NextMemoryLevel::requestWrite(int address, int value, int notBefore) {
	memoryInterface->requestWrite(address, value, notBefore);
}

bool NextMemoryLevel::getWriteResponse(int* address, int notBefore) {
	return memoryInterface->getWriteResponse(address, notBefore);
}
