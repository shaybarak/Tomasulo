#include "Clock.h"

void Clock::tick() {
	// Advance clock
	time++;
	// Notify observers back-to-forward
	for (vector<Clocked*>::iterator it = observers.begin(); it < observers.end(); it++) {
		(*it)->onTick(time);
	}
	// Notify observers forward-to-back (allows passing back values in the same clock cycle)
	// (the last observer is not called to prevent double dispatching)
	for (vector<Clocked*>::reverse_iterator rit = observers.rbegin()+1; rit < observers.rend(); rit++) {
		(*rit)->onTick(time);
	}
}

void Clock::addObserver(Clocked* clocked) {
	observers.push_back(clocked);
}
