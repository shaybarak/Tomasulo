#include "Clock.h"

void Clock::tick() {
	// Advance clock
	time++;
	// Notify observers
	for (vector<Clocked*>::iterator it = observers.begin() ; it < observers.end(); it++) {
		(*it)->onTick(time);
	}
}

void Clock::addObserver(Clocked* clocked) {
	observers.push_back(clocked);
}
