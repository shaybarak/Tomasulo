#include "NextMemoryLevel.h"

void NextMemoryLevel::requestRead(int address, int notBefore) {
	memoryInterface->requestRead(address, notBefore);
}

bool NextMemoryLevel::getReadResponse(int* address, int* data, int notBefore) {
	return memoryInterface->getReadResponse(address, *data, notBefore);
}

void NextMemoryLevel::requestWrite(int address, int data, int notBefore) {
	memoryInterface->requestWrite(address, data, notBefore);
}
