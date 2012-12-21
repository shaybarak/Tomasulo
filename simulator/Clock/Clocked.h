#pragma once

/** Base class for clocked components. */
class Clocked {
public:
	// Clock tick handler
	virtual void onTick(int now) = 0;
};
