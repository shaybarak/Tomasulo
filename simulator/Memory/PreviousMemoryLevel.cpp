#include "PreviousMemoryLevel.h"

void PreviousMemoryLevel::respondRead(int address, int data, int now) {
	memoryInterface->responsdRead(address, data, now);
}

bool PreviousMemoryLevel::getReadRequest(int* address, int now) {
	return memoryInterface->getReadRequest(address, now);
}

bool PreviousMemoryLevel::getWriteRequest(int* address, int* data, int now) {
	return memoryInterface->getWriteRequest(address, data, now);
}
