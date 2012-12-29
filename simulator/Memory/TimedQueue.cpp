/*#include "TimedQueue.h"

template <typename T>
void TimedQueue<T>::push(const T& element, int notBefore) {
	struct TimedQueue::element elem;
	elem.value = element;
	elem.notBefore = notBefore;
	q.push(elem);
}

template <typename T>
bool TimedQueue<T>::pop(T* element, int notAfter) {
	struct TimedQueue::element elem = q.front();
	if (elem.notBefore <= notAfter) {
		*element = elem.value;
		return true;
	} else {
		return false;
	}
}*/
