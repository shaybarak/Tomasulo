#pragma once

#include "Cache.h"

class L1Cache : public Cache {
public:
	virtual bool read(int address, int* value);
	virtual bool write(int address, int value);
};
