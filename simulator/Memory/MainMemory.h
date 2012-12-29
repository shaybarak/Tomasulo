#pragma once

class MainMemory : Clocked {
	MainMemory(int accessDelay, int l2BlockSize, PreviousMemoryLevel* previousMemoryLevel);
	const vector<char>* getBuffer() { return buffer; }
	// WARNING: exposes non-const pointer to data member
	vector<char>* getBuffer() { return buffer; }
	virtual void onTick(int now);

private:
	vector<char> buffer;
	int* words;
	int accessDelay;
	int l2BlockSize;
	// For identifying sequential access
	int lastReadAddress;
	// Busy reading until this time
	int busyReadingUntil;
	PreviousMemoryLevel* previousMemoryLevel;
};
