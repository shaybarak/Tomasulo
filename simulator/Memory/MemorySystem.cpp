#include "MemorySystem.h"
#include <assert.h>

int MemorySystem::read(int now, int address, int& value) {
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
		value = l1->read(address);
		// Delay should be L1 access delay
		return now;
	}

	// If present in L1, register L1 hit and return
	if (l1->isPresent(address)) {
		l1->registerHit();
		value = l1->read(address);
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
		value = l2->read(address);
	}
	
	// If present in L2 (either because pending writes were applied or regardless) register L2 hit
	if (l2->isPresent(address)) {
		l2->registerHit();
		value = l2->read(address);
		
		// Write block from L2 to L1, critical word first
		for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
			PendingWrite pending;
			pending.when = now + i;  // Data is read sequentially from L2 to L1 (one word every cycle)
			pending.address = address;
			pending.value = l2->read(address);
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

	value = criticalWord;
	return now;
}

int MemorySystem::write(int now, int address, int value) {

	// If buffer is not empty, delay by L1 access time
	if (!isWriteBufferEmpty(now)) {
		now += l1->getAccessDelay();
	}

	// If pending write then delay until applied, register L1 hit and return
	PendingWrite pending = findPendingWrite(l1PendingWrites, address);
	if (pending.when >= 0) {
		l1->registerHit();
		now = pending.when;
		applyPendingWrites(now);
		value = l1->read(address);
		// Delay should be L1 access delay
		return now;
	}

	// If present in L1, done. Register L1 hit. Apply write.
	if (l1->isPresent(address)) {
		l1->registerHit();
		// Apply all pending operations until this time
		applyPendingWrites(now);
		return now;
	}
	// Address is not present in L1. Register L1 miss.
	l1->registerMiss();
	
	// If L2-RAM interface busy, delay until free
	if (l2RamInterfaceBusyUntil > now) {
		now = l1L2InterfaceBusyUntil;
		applyPendingWrites(now);
	}

	// Delay by L2 access time.
	now += l2->getAccessDelay();
	applyPendingWrites(now);
	
	// If pending write then delay until applied
	pending = findPendingWrite(l2PendingWrites, address);
	if (pending.when >= 0) {
		now = pending.when;
		applyPendingWrites(now);
		value = l2->read(address);
	}

	// Search in L2 ways
	int destinationWay = l2->getPresentWay(address);
	
	// If address is present in l2
	if (destinationWay != -1) {
		//register L2 hit. 
		l2->registerHit();
		//WRITE ALLOCATE: first copy block to L1, then write word to both L1 and L2
		int copyAddress = address;
		for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
			PendingWrite pending;
			pending.when = now + i;  // Data is read sequentially from L2 to L1 (one word every cycle)
			pending.address = copyAddress + i * sizeof(int);
			pending.value = l2->read(pending.address);
			pending.way = destinationWay;
			pending.dirty = false;
			l1PendingWrites.push_back(pending);
			// Make sure we know that the L1-L2 interface is busy
			l1L2InterfaceBusyUntil = pending.when;
		}
		//finally, commit the write to both L1 and L2
		PendingWrite pendingSw;
		pendingSw.address = address;
		pendingSw.value = value;
		pendingSw.way = destinationWay;
		pendingSw.dirty = true;
		pendingSw.when = now + l1->getBlockSize() / sizeof(int);
		l1PendingWrites.push_back(pendingSw);
		l2PendingWrites.push_back(pendingSw);
		l1L2InterfaceBusyUntil = pendingSw.when;
		return now;
	}
	
	//Oh-no! miss both in L2 and L1! need to call ram for help
	now += ram->getAccessDelay();
	applyPendingWrites(now);
	l2RamInterfaceBusyUntil = now + l2->getBlockSize() / sizeof(int) - 1;
	// Decide which way: first, take the invalid way
	destinationWay = l2->getInvalidWay(address);
	// Decide which way: second, take LRU way
	if (destinationWay == -1) {
		destinationWay = l2->getLruWay(address);
	}
	//Invalidate all l1 blocks (there is a function) remeber to bring all something.
	int baseL2Address = 
		l2->getConflictingAddress(address, destinationWay) / l2->getBlockSize() * l2->getBlockSize();
	for (int i = 0; i < l2->getBlockSize(); i+=sizeof(int)) {
		l1->invalidate(baseL2Address + i);
	}
	//Critical L1 Block first : copy to both L1 and L2, every word is written in the same time. 
	int baseL1Address = address / l1->getBlockSize() * l1->getBlockSize();
	for (unsigned int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
		PendingWrite pendingL1, pendingL2;
		pendingL1.address = baseL1Address + i * sizeof(int);
		pendingL1.value = ram->read(pendingL1.address);
		pendingL1.when = now + i;
		l1PendingWrites.push_back(pendingL1);
		pendingL2.address = baseL2Address + i * sizeof(int);
		pendingL2.value = ram->read(pendingL1.address);
		pendingL2.when = now + i;
		pendingL2.way = destinationWay;
		pendingL2.dirty = false;
		l2PendingWrites.push_back(pendingL2);
	}
	
	// finally, commit word to L1, L2
	PendingWrite pendingL1, pendingL2;
	pendingL1.address = baseL1Address + l1->getBlockSize();
	pendingL1.value = value;
	pendingL1.when = now + l1->getBlockSize() / sizeof(int);
	l1PendingWrites.push_back(pendingL1);
	
	pendingL2.address = baseL2Address + l2->getBlockSize();
	pendingL2.value = value;
	pendingL2.when = pendingL1.when;
	pendingL2.way = destinationWay;
	pendingL2.dirty = true;
	l2PendingWrites.push_back(pendingL2);
	l1L2InterfaceBusyUntil = now + l1->getBlockSize() / sizeof(int);
	
	now += l1->getBlockSize();
	applyPendingWrites(now);
	// Copy rest of L2 block to L2. assumes cyclic.
	int l2WriteAddress = baseL2Address + l1->getBlockSize();
	int wordCount = (l2->getBlockSize() - l1->getBlockSize()) / sizeof(int);
	for (int i = 0; i < wordCount; i++) {
		PendingWrite pending;
		pending.when = now + i;
		pending.value = ram->read(l2WriteAddress);
		pending.address = l2WriteAddress;
		pending.way = destinationWay;
		l2PendingWrites.push_back(pending);
		l2WriteAddress = nextAddress(l2WriteAddress, l2->getBlockSize());
		// Make sure we know that the L2-RAM interface is busy
		l2RamInterfaceBusyUntil = pending.when;
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
