#pragma once

class MainMemory : Clocked {
	MainMemory(int accessDelay, PreviousMemoryLevel* previousMemoryLevel);
	const vector<char>* getBuffer() { return buffer; }
	// WARNING: exposes non-const pointer to data member
	vector<char>* getBuffer() { return buffer; }
	virtual void onTick(int now);

private:
	vector<char> buffer;
	int* words;
	int accessDelay;
	PreviousMemoryLevel* previousMemoryLevel;
};
