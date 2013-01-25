#include "MemorySystem.h"
#include <assert.h>

int MemorySystem::read(int now, int address, int& value) {
	value = ram->read(address);

	// Delay by L1 access time
	now += l1->getAccessDelay();
	// Apply all pending operations until this time
	applyPendingWrites(now);

	// If pending write then delay until applied, register L1 hit and return
	PendingWrite pending = findPendingWrite(l1PendingWrites, address);
	if (pending.when >= 0) {
		l1->registerHit();
		now = pending.when;
		applyPendingWrites(now);
		//value = l1->read(address);
		// Delay should be L1 access delay
		return now;
	}

	// If present in L1, register L1 hit and return
	if (l1->isPresent(address)) {
		l1->registerHit();
		//value = l1->read(address);
		// Delay should be L1 access delay
		return now;
	}

	// Else register L1 miss.
	l1->registerMiss();

	// If L1-L2 interface busy, delay until free
	if (l1L2InterfaceBusyUntil > now) {
		now = l1L2InterfaceBusyUntil;
		applyPendingWrites(now);
	}

	// Delay by L2 access time
	now += l2->getAccessDelay();
	applyPendingWrites(now);

	// If pending write then delay until applied
	pending = findPendingWrite(l2PendingWrites, address);
	if (pending.when >= 0) {
		now = pending.when;
		applyPendingWrites(now);
		//value = l2->read(address);
	}

	// If present in L2 (either because pending writes were applied or regardless) register L2 hit
	if (l2->isPresent(address)) {
		l2->registerHit();
		//value = l2->read(address);

		// Write block from L2 to L1, critical word first
		for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
			PendingWrite pending;
			pending.when = now + i;  // Data is read sequentially from L2 to L1 (one word every cycle)
			pending.address = address;
			//pending.value = l2->read(address);
			address = nextAddress(address, l1->getBlockSize());
			l1PendingWrites.push_back(pending);
			// Make sure we know that the L1-L2 interface is busy
			l1L2InterfaceBusyUntil = pending.when;
		}

		// Delay is L1 access + L2 access + time waiting for pending writes if relevant
		return now;
	}

	// Else register L2 miss.
	l2->registerMiss();

	// If L2-RAM interface busy, delay until free
	if (l2RamInterfaceBusyUntil > now) {
		now = l1L2InterfaceBusyUntil;
		applyPendingWrites(now);
	}

	// Find destination way (preferably an invalid way so as not to have to evict)
	int destinationWay = l2->getInvalidWay(address);
	if (destinationWay < 0) {
		// Conflict, evict from LRU way
		destinationWay = l2->getLruWay(address);
		int conflictingAddressBase = l2->getConflictingAddress(address, destinationWay) / l2->getBlockSize() * l2->getBlockSize();
		int conflictingAddress = conflictingAddressBase;
		// Invalidate L2 block and all contained L1 blocks
		for (int i = 0; i < l2->getBlockSize() / /*l1->getBlockSize()*/ sizeof(int); i++) {
			l1->invalidate(conflictingAddress);
			conflictingAddress += /*l1->getBlockSize()*/ sizeof(int);
		}
		// If conflicting block is dirty
		if (l2->isDirty(conflictingAddressBase)) {
			// Write back to RAM
			int conflictingAddress = conflictingAddressBase;
			for (int i = 0; i < l2->getBlockSize() / sizeof(int); i++) {
				ram->write(conflictingAddress, l2->read(conflictingAddress));
				conflictingAddress += sizeof(int);
			}
			// Delay for writing to RAM
			now += ram->getAccessDelay();  // Initial row access
			now += l2->getBlockSize() / sizeof(int) - 1;  // One cycle for each additional access
		}
	}

	// Bring data from RAM:
	// Critical word first,
	int criticalWord = ram->read(address);
	now += ram->getAccessDelay();
	applyPendingWrites(now);
	l1->write(address, criticalWord);
	l2->write(address, criticalWord, destinationWay, true);

	// Then critical L1 block,
	int pendingTime = now;
	for (int i = 1; i < l1->getBlockSize() / sizeof(int); i++) {
		pendingTime++;
		pending.when = pendingTime;
		address = nextAddress(address, l1->getBlockSize());
		pending.address = address;
		pending.value = ram->read(address);
		pending.dirty = false;
		pending.way = destinationWay;
		l1PendingWrites.push_back(pending);
		l2PendingWrites.push_back(pending);
	}

	// Then rest of L2 block.
	address = address / l2->getBlockSize() * l2->getBlockSize() + ((address / l1->getBlockSize() * l1->getBlockSize() + l1->getBlockSize()) % l2->getBlockSize());
	for (int i = 0; i < (l2->getBlockSize() - l1->getBlockSize()) / sizeof(int); i++) {
		pendingTime++;
		pending.when = pendingTime;
		address = nextAddress(address, l2->getBlockSize());
		pending.address = address;
		pending.value = ram->read(address);
		pending.dirty = false;
		pending.way = destinationWay;
		l2PendingWrites.push_back(pending);
	}
	// Keep L2-RAM interface busy until done transferring last word
	l2RamInterfaceBusyUntil = pendingTime;

	//value = criticalWord;
	return now;
}

void MemorySystem::pushWriteThrough(int address, int value, int way, int now) {
	PendingWrite pending;
	pending.address = address;
	pending.value = value;
	pending.way = way;
	assert(pending.way >= 0);
	pending.when = now;
	pending.dirty = true;
	l1PendingWrites.push_back(pending);
	l2PendingWrites.push_back(pending);
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

		//INVALIDATRE l1 blocks
		conflictingAddress = conflictingAddress / l2->getBlockSize() * l2->getBlockSize();
		for (int i = 0; i < l2->getBlockSize(); i+=sizeof(int)) {
			l1->invalidate(conflictingAddress );
			conflictingAddress += sizeof(int);
		}
	}

	//TODO: Critical L1 Block first : copy to both L1 and L2, every word is written in the same time. 
	//Copy L2 Block
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
