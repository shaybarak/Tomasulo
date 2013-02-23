#include "MemorySystem.h"
#include <assert.h>

int MemorySystem::read(int now, int address, int& value) {
	if (now < l1BusyUntil) {
		// Wait for L1 to become free for access
		now = l1BusyUntil;
	}

	// Delay by L1 access time
	now += l1->getAccessDelay();

	// If present in L1, register L1 hit and return
	if (l1->isPresent(address)) {
		l1->registerHit();
		value = l1->read(address);
		// Delay should be L1 access delay
		l1BusyUntil = now;
		return now;
	}

	// Else register L1 miss.
	l1->registerMiss();

	// Delay by L2 access time
	now += l2->getAccessDelay();

	// If present in L2 register L2 hit
	if (l2->isPresent(address)) {
		l2->registerHit();
		value = l2->read(address);
		
		// Write block from L2 to L1, critical word first
		for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
			l1->write(address, l2->read(address));
			address = nextAddress(address, l1->getBlockSize());
		}
		// Pay one cycle penalty for every word except for the first
		//now += l1->getBlockSize() / sizeof(int) - 1;

		// Delay is L1 access + L2 access + time waiting for pending writes if relevant
		l1BusyUntil = now;
		return now;
	}

	// Else register L2 miss.
	l2->registerMiss();

	// Find destination way (preferably an invalid way to avoid eviction if possible)
	int destinationWay = l2->getInvalidWay(address);
	if (destinationWay < 0) {
		// Conflict, evict from LRU way
		destinationWay = l2->getLruWay(address);
		int conflictingAddressBase = l2->getConflictingAddress(address, destinationWay) / l2->getBlockSize() * l2->getBlockSize();
		int conflictingAddress = conflictingAddressBase;
		// Invalidate all contained L1 blocks
		for (int i = 0; i < l2->getBlockSize() / l1->getBlockSize(); i++) {
			l1->invalidate(conflictingAddress);
			conflictingAddress += l1->getBlockSize();
		}
		// If conflicting block is dirty
		if (l2->isDirty(conflictingAddressBase)) {
			// Write back to RAM
			int conflictingAddress = conflictingAddressBase;
			now += ram->getAccessDelay();  // Initial row access
			for (unsigned int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {
				ram->write(conflictingAddress, l2->read(conflictingAddress));
				conflictingAddress += sizeof(int);
			}
			// First L2->RAM transfer was already paid for by RAM access delay
			now += l2->getBlockSize() / sizeof(int) - 1;
		}
	}

	// Bring data from RAM:
	// Critical word first,
	value = ram->read(address);
	now += ram->getAccessDelay();
	l1->write(address, value);
	l2->write(address, value, destinationWay, false);

	// Then critical L1 block,
	for (unsigned int i = 1; i < l1->getBlockSize() / sizeof(int); i++) {
		address = nextAddress(address, l1->getBlockSize());
		int data = ram->read(address);
		l1->write(address, data);
		l2->write(address, data, destinationWay, false);
	}
	//now += l1->getBlockSize() / sizeof(int) - 1;

	// Then entire L2 block
	address = address / l2->getBlockSize() * l2->getBlockSize();
	for (unsigned int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {
		int data = ram->read(address);
		l2->write(address, data, destinationWay, false);
		address = nextAddress(address, l2->getBlockSize());
	}
	//now += (l2->getBlockSize() - l1->getBlockSize()) / sizeof(int);

	l1BusyUntil = now;
	return now;
}

int MemorySystem::write(int now, int address, int value) {
	int oldValue;
	now = read(now, address, oldValue);
	l1->write(address, value);
	l2->write(address, value, l2->getPresentWay(address), true);
	l1BusyUntil = now;
	return now;
}

void MemorySystem::applyAllPendingWrites() {
	applyPendingWrites(INT_MAX);
}

void MemorySystem::resetTiming() {
	l1BusyUntil = -1;
	l1L2InterfaceBusyUntil = -1;
	l2RamInterfaceBusyUntil = -1;
}

void MemorySystem::applyPendingWrites(int until) {
	PendingWrite write;
	// Commit all pending writes to L1
	for (vector<PendingWrite>::iterator it = l1PendingWrites.begin(); it < l1PendingWrites.end(); it++) {
		write = *it;
		if (write.when <= until) {
			l1->write(write.address, write.value);
		}
	}
	// Commit all pending writes to L2
	for (vector<PendingWrite>::iterator it = l2PendingWrites.begin(); it < l2PendingWrites.end(); it++) {
		write = *it;
		// Validate way
		assert((write.way == 0) || (write.way == 1));
		if (write.when <= until) {
			l2->write(write.address, write.value, write.way, write.dirty);
		}
	}
}

MemorySystem::PendingWrite MemorySystem::findPendingWrite(vector<PendingWrite>& pending, int address) {
	PendingWrite write;
	for (vector<PendingWrite>::iterator it = pending.begin(); it < pending.end(); it++) {
		write = *it;
		if (write.address == address) {
			// Assumes no more than one pending write per address
			return write;
		}
	}
	// Not found, mark as invalid and return
	write.when = -1;
	return write;
}

int MemorySystem::nextAddress(int address, int blockSize) {
	return
		// Base of block
		((address / blockSize) * blockSize) +
		// New offset, incremented by one word
		((address + sizeof(int)) % blockSize);
}

bool MemorySystem::isWriteBufferEmpty(int now) {
	return now >= l1L2InterfaceBusyUntil;
}
