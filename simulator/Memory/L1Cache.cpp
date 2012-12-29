#include "L1Cache.h"

void L1Cache::onTick(int now) {
	// TODO
}

bool L1Cache::read(int offset, int* value) {
	if (offset < cacheSize / 2) {
		// Use instructions buffer
		*value = instructions[offset / sizeof(int)]
		return instructionsValid[offset / blockSize];
	} else {
		offset = offset % (cacheSize / 2);
		*value = data[(offset / sizeof(int)]
		return dataValid[offset / blockSize];
	}
}

void L1Cache::write(int offset, int value) {
	if (offset < cacheSize / 2) {
		// Use instructions buffer
		instructions[offset / sizeof(int)] = value;
	} else {
		offset = offset % (cacheSize / 2);
		data[(offset / sizeof(int)] = value;
	}
}
