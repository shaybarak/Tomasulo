#pragma once

#include <vector>
using namespace std;

/** A system clock. */
class Clock {
public:
	// Initializes clock with start time of 0
	Clock() : time(0) {}
	// Initializes clock with a given start time
	Clock(int time) : time(time) {}
	// Register a new clocked component
	void register(Clocked* clocked);
	// Generate a clock tick and notify all clocked components
	void tick();
	// Returns the current time
	void getTime() { return time; }

private:
	int time;
	vector<Clocked*> observers;
};
