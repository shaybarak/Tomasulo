#pragma once

/** Base class for clocked components. */
class Clocked {
public:
	// Clock tick handler for reading signals from slave and sending signals to master
	virtual void onTickDown(int now) = 0;
	// Clock tick handler for reading signals from master and sending signals to slave
	virtual void onTickUp(int now) = 0;
};
