#include "MemorySystem.h"

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
	PendingWrite pending = findPendingWrite(l2PendingWrites, address);
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
		for (int i = 0; i < l1->getBlockSize() / sizeof(int); i++) {
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

	// Delay by RAM access time (RAM row is guaranteed not to be open when in L2 miss)
	now += ram->getAccessDelay();
	applyPendingWrites(now);

	// If L2 conflict, choose way to evict (prefer LRU). TODO make sure that way0IsLru is initialized to true. Alternatively have lru = vector<int>.
	// If way to evict is valid, immediately invalidate L2 block and any matching L1 blocks.
	// (memory system is busy anyway, doesn't matter when we apply the invalidation)
	// If way to evict is valid and dirty, write block to RAM.
	// Register pending writes to L1, critical word first. Register same writes to L2 at same times (inclusive).
	// Register pending writes to L2, rest of L2 block.
	// Update time until L2-RAM interface is free (by last word).
	// Return time until critical word first is available.
	return now;
}

int MemorySystem::write(int now, int address, int value) {
	//TODO: dirty, LRU
	// If buffer is not empty, delay by L1 access time
	if (!isWriteBufferEmpty(now)) {
		now += l1->getAccessDelay();
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
	// Delay by L2 access time.
	now += l2->getAccessDelay();
	// Search in L2 ways
	int destinationWay = -1;
	for (int way = 0; way < l2->getWayCount(); way ++) {
		if (l2->isPresentInWay(address, 0)) {
			destinationWay = way;
			break;
		}
	}
	// If present in L2, register L2 hit. 
	if (destinationWay != -1) {
		l2->registerHit();
		//Register first write
		PendingWrite pending;
		pending.address = address;
		pending.value = value;
		pending.when = now;


		//Register pending writes to L1, Critical Word first
		int blockBase = address / l2->getBlockSize() * l2->getBlockSize();
		for (int wordIndex = 0; wordIndex < l1->getBlockSize(); wordIndex += sizeof(int)) {
			int cyclicAddress = address + wordIndex;

			PendingWrite pending;
			pending.address = cyclicAddress;
		}
	}
	
	// Register pending write to L1 and L2 (same time since inclusive) to apply write.
	// Else check L2 pending, if in pending then additional wait and OH FUCK IT LET'S CODE READ AND TEST IT FIRST.
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
		if (write.when <= until) {
			l2->write(write.address, write.value);
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
