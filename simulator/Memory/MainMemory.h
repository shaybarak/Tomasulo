#pragma once

class MainMemory : Clocked {
	MainMemory(int* buffer, int accessDelay,
		PreviousMemoryLevel* previousMemoryLevel)
		: buffer(buffer), accessDelay(accessDelay), previousMemoryLevel(previousMemoryLevel) {}
	virtual void onTick(int now);

private:
	int* buffer;
	int accessDelay;
	PreviousMemoryLevel* previousMemoryLevel;
};
