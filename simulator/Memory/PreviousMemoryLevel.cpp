#include "PreviousMemoryLevel.h"

bool PreviousMemoryLevel::getReadRequest(int* address, int notBefore) {
	return memoryInterface->getReadRequest(address, notBefore);
}

void PreviousMemoryLevel::respondRead(int address, int value, int notBefore) {
	memoryInterface->respondRead(address, value, notBefore);
}

bool PreviousMemoryLevel::getWriteRequest(int* address, int* value, int notBefore) {
	return memoryInterface->getWriteRequest(address, value, notBefore);
}

void PreviousMemoryLevel::respondWrite(int address, int notBefore) {
	memoryInterface->respondWrite(address, notBefore);
}
