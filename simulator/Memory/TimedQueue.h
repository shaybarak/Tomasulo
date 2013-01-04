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

template <typename T>
void TimedQueue<T>::push(const T& element, int notBefore) {
	struct TimedQueue::element elem;
	elem.value = element;
	elem.notBefore = notBefore;
	q.push(elem);
}

template <typename T>
bool TimedQueue<T>::pop(T* element, int notAfter) {
	if (q.empty()) {
		return false;
	}
	struct TimedQueue::element elem = q.front();
	if (elem.notBefore <= notAfter) {
		*element = elem.value;
		q.pop();
		return true;
	} else {
		return false;
	}
}