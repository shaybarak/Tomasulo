#include "PreviousMemoryLevel.h"

void PreviousMemoryLevel::respondRead(int address, int data, int notBefore) {
	memoryInterface->respondRead(address, data, notBefore);
}

bool PreviousMemoryLevel::getReadRequest(int* address, int notBefore) {
	return memoryInterface->getReadRequest(address, notBefore);
}

bool PreviousMemoryLevel::getWriteRequest(int* address, int* data, int notBefore) {
	return memoryInterface->getWriteRequest(address, *data, notBefore);
}
