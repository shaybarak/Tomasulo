#pragma once

#include "Cache.h"

class L2Cache : public Cache, public Clocked {
public:
	virtual void onTick(int now);
};
