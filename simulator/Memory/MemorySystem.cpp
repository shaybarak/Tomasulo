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
		return now;
	}

	// If present in L1, register L1 hit and return
	if (l1->isPresent(address)) {
		l1->registerHit();
		value = l1->read(address);
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

	// If pending write then delay until applied, register L2 hit
	PendingWrite pending = findPendingWrite(l2PendingWrites, address);
	if (pending.when >= 0) {
		l2->registerHit();
		now = pending.when;
		applyPendingWrites(now);
		value = l2->read(address);
	} else {
		// If present in L2, register L2 hit.
		l2->registerHit();
		value = l2->read(address);
	}
	
	// Write block from L2 to L1, critical word first
	//for (int i = 0; i < l1->get

	//   Register pending writes to L1, critical word first.
	//	 Update time until L1-L2 interface is free (by last word, not by critical word).
	//   Return (with critical word time which is L1 access + L2 access + pending delay).
	// Else register L2 miss.
	// If L2-RAM interface busy, delay further.
	// Delay by RAM access time (note row will always be closed at this time).
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
	// Delay by L1 access time
	now += l1->getAccessDelay();
	// Apply all pending operations until this time
	applyPendingWrites(now);
	// Delay by L1 access time.
	// If write buffer is busy (evident by L1-L2 interface busy), delay further.
	// If present in L1, done. Register L1 hit. Apply write.
	// Else register L1 miss.
	// Delay by L2 access time.
	// If present in L2, register L2 hit. Register pending writes to L1, normal order.
	// Register pending write to L1 and L2 (same time since inclusive) to apply write.
	// Else check L2 pending, if in pending then additional wait and OH FUCK IT LET'S CODE READ AND TEST IT FIRST.
	return now;
}

void MemorySystem::applyAllPendingWrites() {

}

void MemorySystem::applyPendingWrites(int until) {

}

MemorySystem::PendingWrite MemorySystem::findPendingWrite(vector<PendingWrite>& pending, int address) {
	PendingWrite out;
	for (vector<PendingWrite>::iterator it = pending.begin(); it < pending.end(); it++) {
		out = *it;
		if (out.address == address) {
			// Assumes no more than one pending write per address
			return out;
		}
	}
	// Not found, mark as invalid and return
	out.when = -1;
	return out;
}
