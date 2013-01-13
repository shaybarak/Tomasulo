#include "L1Cache.h"
#include "../MIPS32/ISA.h"
#include <assert.h>

bool L1Cache::isPresent(int address) {
	int index = toIndex(address);
	int tag = toTag(address);
	return (valid[index] && (tags[index] == tag));
}

int L1Cache::read(int address) {
	int index = toIndex(address);
	int offset = toOffset(address);
	return *getWordPtr(index, offset);
}

void L1Cache::write(int address, int value, int way_unused) {
	int tag = toTag(address);
	int index = toIndex(address);
	int offset = toOffset(address);
	*getWordPtr(index, offset) = value;
	valid[index] = true;
	tags[index] = tag;
}

bool L1Cache::invalidate(int address) {
	if (isPresent(address)) {
		valid[toIndex(address)] = false;
		return true;
	}
	return false;
}
