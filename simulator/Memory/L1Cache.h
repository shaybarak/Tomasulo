#pragma once

#include "Cache.h"
#include "../Clock/Clocked.h"

class L1Cache : public Cache, public Clocked {
public:
	virtual void onTick(int now);
};
