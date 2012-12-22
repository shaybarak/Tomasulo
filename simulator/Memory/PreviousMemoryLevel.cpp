#include "PreviousMemoryLevel.h"

template <typename AddressType, typename DataType>
void PreviousMemoryLevel::respondRead(AddressType address, DataType data, int now) {
	memoryInterface->responsdRead(address, data, now);
}

template <typename AddressType, typename DataType>
bool PreviousMemoryLevel::getReadRequest(AddressType* address, int now) {
	return memoryInterface->getReadRequest(address, now);
}

template <typename AddressType, typename DataType>
bool PreviousMemoryLevel::getWriteRequest(AddressType* address, DataType* data, int now) {
	return memoryInterface->getWriteRequest(address, data, now);
}
