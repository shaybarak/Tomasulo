#include "MemorySystem.h"
#include <assert.h>

int MemorySystem::read(int now, int address, int& value) {
	// Delay by L1 access time
	now += l1->getAccessDelay();

	// If present in L1, register L1 hit and return
	if (l1->isPresent(address)) {
		l1->registerHit();
		value = l1->read(address);
		// Delay should be L1 access delay
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
		now += l1->getBlockSize() / sizeof(int) - 1;

		// Delay is L1 access + L2 access + time waiting for pending writes if relevant
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
			for (int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {
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
	for (int i = 1; i < l1->getBlockSize() / sizeof(int); i++) {
		address = nextAddress(address, l1->getBlockSize());
		int data = ram->read(address);
		l1->write(address, data);
		l2->write(address, data, destinationWay, false);
	}
	now += l1->getBlockSize() / sizeof(int) - 1;

	// Then entire L2 block
	address = address / l2->getBlockSize() * l2->getBlockSize();
	for (int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {
		int data = ram->read(address);
		l2->write(address, data, destinationWay, false);
		address = nextAddress(address, l2->getBlockSize());
	}
	now += (l2->getBlockSize() - l1->getBlockSize()) / sizeof(int);

	return now;
}

int MemorySystem::write(int now, int address, int value) {

	//TODO: if write buffer is empty?
	if (!isWriteBufferEmpty(now)) {
		now += l1->getAccessDelay();
	}

	/////////////////////////////////////////////
	//L1 HIT
	/////////////////////////////////////////////
	if (l1->isPresent(address)) {
		l1->registerHit();
		// Apply all pending operations until this time
		l1->write(address, value);
		l2->write(address, value, l2->getPresentWay(address), true); //TODO: do we need isDirty??
		return now;
	}

	////////////////////////////////////////////////
	//L1 MISS
	///////////////////////////////////////////////
	l1->registerMiss();
	now += l2->getAccessDelay();
	/////////////////////////////////////////////
	//L2 HIT
	////////////////////////////////////////////
	if (l2->isPresent(address)) {
		l2->registerHit();
		//write allocate
		int copyAddress = address / l1->getBlockSize() * l1->getBlockSize();
		for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {			
			l1->write(copyAddress, l2->read(copyAddress));
			copyAddress += sizeof(int);
		}
		now += l1->getBlockSize()/sizeof(int) - 1;
		//write data to both l1, l2
		l1->write(address, value);
		l2->write(address, value, l2->getPresentWay(address), true);
		return now;
	}

	/////////////////////////////////////////////
	//L2 MISS
	////////////////////////////////////////////
	l2->registerMiss();
	now += ram->getAccessDelay();

	int destinationWay = l2->getInvalidWay(address);
	if (destinationWay == -1) {
		//WRITEBACK conflicting L2 block to RAM
		destinationWay = l2->getLruWay(address);
		int conflictingAddress = l2->getConflictingAddress(address, destinationWay) / l2->getBlockSize() * l2->getBlockSize();
		for (unsigned int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {			
			ram->write(conflictingAddress, l2->read(conflictingAddress));
			conflictingAddress += sizeof(int);
		}
		now += l2->getBlockSize() / sizeof(int) - 1;

		//INVALIDATE l1 blocks
		conflictingAddress = conflictingAddress / l2->getBlockSize() * l2->getBlockSize();
		for (int i = 0; i < l2->getBlockSize(); i+=sizeof(int)) {
			l1->invalidate(conflictingAddress );
			conflictingAddress += sizeof(int);
		}
	}

	//TODO: Critical L1 Block first : copy to both L1 and L2, every word is written in the same time. 
	//Copy L2 Block from RAM
	int l2CopyAddress = address / l2->getBlockSize() * l2->getBlockSize();
	for (unsigned int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {			
		l2->write(l2CopyAddress, ram->read(address), destinationWay, false);
		l2CopyAddress += sizeof(int);
	}
	now += l2->getBlockSize() / sizeof(int) - 1;

	// Copy L1 block from L2
	int l1CopyAddress = address / l1->getBlockSize() * l1->getBlockSize();
	for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
		l1->write(l1CopyAddress, l2->read(l1CopyAddress));
		address += sizeof(int);
	}
	return now;
}

void MemorySystem::applyAllPendingWrites() {
	applyPendingWrites(INT_MAX);
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
