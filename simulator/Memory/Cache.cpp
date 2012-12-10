#include "Cache.h"

Cache::Cache(Memory* nextLevel, int sizeInBytes, int accessDelay, int sizeOfBlock) : Memory(sizeInBytes, accessDelay) {
  	this.nextLevel = nextLevel;
  	this.sizeOfBlock = sizeOfBlock;
	// Initialize tags to invalids
	tags.resize(sizeInBytes / sizeOfBlock, INVALID);
}