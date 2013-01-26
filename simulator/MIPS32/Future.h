#pragma once

template<typename T> class Future {
public:
	Future(T& value, int notBefore) : value(value), notBefore(notBefore) {}
	bool isReady(int now) { return now >= notBefore; }
	T& get() { return value; }
private:
	// Future value to keep
	T value;
	// Value is available not before given time
	int notBefore;
};
