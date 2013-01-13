#include "MemorySystem.h"

int MemorySystem::read(int now, int address, int* value) {
	// Apply all pending operations until this time
	applyAllPending(now);
	int delay = 0;
	// Delay by L1 access time.
	// If present in L1, register L1 hit and return.
	// Else if pending then delay until applied, register L1 hit and return.
	// Else register L1 miss.
	// If L1-L2 interface busy, delay until free.
	// Delay by L2 access time.
	// If present in L2 or pending, register L2 hit.
	//   If pending then delay until applied.
	//   Register pending writes to L1, critical word first.
	//   Return.
	// Else register L2 miss.
	// If L2-RAM interface busy, delay further.
	// Delay by RAM access time (note row will always be closed at this time).
	// If L2 conflict, choose way to evict (prefer LRU, then 0).
	// If way to evict is valid, immediately invalidate L2 block and any matching L1 blocks.
	// (memory system is busy anyway, doesn't matter when we apply the invalidation)
	// If way to evict is valid and dirty, register pending L2 writes to RAM (use open row delay).
	// Register pending writes to L1, critical word first. Register same writes to L2 at same times (inclusive).
	// Register pending writes to L2, rest of L2 block.
	return now + delay;
}

int MemorySystem::write(int now, int address, int value) {
	// Apply all pending operations until this time
	applyAllPending(now);
	int delay = 0;
	// Delay by L1 access time.
	// If write buffer is busy (evident by L1-L2 interface busy), delay further.
	// If present in L1, done. Register L1 hit. Apply write.
	// Else register L1 miss.
	// Delay by L2 access time.
	// If present in L2, register L2 hit. Register pending writes to L1, normal order.
	// Register pending write to L1 and L2 (same time since inclusive) to apply write.
	// Else check L2 pending, if in pending then additional wait and OH FUCK IT LET'S CODE READ AND TEST IT FIRST.
	return now + delay;
}

void applyAllPending(int until) {

}
