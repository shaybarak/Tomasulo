#include "NextMemoryLevel.h"

template <typename AddressType, typename DataType>
void NextMemoryLevel::requestRead(AddressType address, int now);
	memoryInterface->responsdRead(address, data, now);
}

template <typename AddressType, typename DataType>
bool NextMemoryLevel::getReadResponse(AddressType* address, DataType& data, int now) {
	return memoryInterface->getReadResponse(address, data, now);
}

template <typename AddressType, typename DataType>
void NextMemoryLevel::requestWrite(AddressType address, DataType data, int now) {
	memoryInterface->requestWrite(address, data, now);
}
