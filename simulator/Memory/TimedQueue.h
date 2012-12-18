#pragma once

#include <queue>
using namespace std;

template <typename T>
class TimedQueue {
public:
	// Push element to be popped not before a given time
	// (assumes elements are pushed in strongly monotonically ascending times)
	void push(const T& element, int notBefore);
	// Pop element if one is available for current time, returns whether element returned
	bool pop(T* element, int notAfter);

private:
	struct element {
		T value;
		int notBefore;
	};
	queue<struct element> q;
};
