#pragma once

#include "L1Cache.h"
#include "L2Cache.h"
#include "MainMemory.h"
#include <vector>

using namespace std;

class MemorySystem {
public:
	MemorySystem(L1Cache* l1, L2Cache* l2, MainMemory* ram) :
		l1(l1), l2(l2), ram(ram),
		l1L2InterfaceBusyUntil(-1), l2RamInterfaceBusyUntil(-1) {}

	/**
	 * Requests a read from the memory system.
	 * now: current time.
	 * address: requested address.
	 * value: output parameter for value read.
	 * Returns the absolute time when this operation will be fulfilled
	 * and the value will be returned to the caller.
	 */
	int read(int now, int address, int* value);

	/**
	 * Requests a write from the memory system.
	 * now: current time.
	 * address: requested address.
	 * value: word to write to memory.
	 * Returns the absolute time when the memory system will unblock
	 * even if the write has not yet been fully committed.
	 */
	int write(int now, int address, int value);

	/**
	 * Applies pending operations.
	 * until: applies all pending operations until this time.
	 */
	void applyPending(int until);

private:
	// Memory components to use
	L1Cache* l1;
	L2Cache* l2;
	MainMemory* ram;

	// When the interfaces between memory components become available
	int l1L2InterfaceBusyUntil;
	int l2RamInterfaceBusyUntil;

	// Pending writes
	typedef struct PendingWrite {
		int when;  // When to apply the change
		int address;
		int value;
	} PendingWrite;
	vector<PendingWrite> l1PendingWrites;
	vector<PendingWrite> l2PendingWrites;
	vector<PendingWrite> ramPendingWrites;
}
