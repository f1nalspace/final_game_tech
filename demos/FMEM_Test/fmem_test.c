/*
-------------------------------------------------------------------------------
Name:
	FMEM | Test

Description:
	This demo shows how to use the "Final Memory" library.

Requirements:
	- C99

Author:
	Torsten Spaete

Changelog:
	## 2018-06-29
	- Initial version
-------------------------------------------------------------------------------
*/

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define fmemAlwaysAssert(exp) if(!(exp)) {*(int *)0 = 0;}

static void TestTemporary() {
	{
		uint8_t *data;

		fmemMemoryBlock block;
		fmemAlwaysAssert(fmemInit(&block, fmemType_Fixed, 1024, 0));

		data = fmemPush(&block, 32, fmemPushFlags_None);
		size_t savedUsed = block.used;

		fmemMemoryBlock temp;
		fmemAlwaysAssert(fmemBeginTemporary(&block, &temp));
		fmemAlwaysAssert(block.temporary == &temp);
		fmemAlwaysAssert(block.size == block.used);
		data = fmemPush(&temp, 256, fmemPushFlags_Clear);
		fmemAlwaysAssert(data != fmem_null);

		fmemAlwaysAssert(fmemPush(&block, 64, fmemPushFlags_None) == fmem_null);

		data = fmemPush(&temp, 1024, fmemPushFlags_None);
		fmemAlwaysAssert(data == fmem_null);

		size_t remaining = fmemGetRemainingSize(&temp);
		data = fmemPush(&temp, remaining, fmemPushFlags_None);
		fmemAlwaysAssert(temp.used == temp.size);

		fmemEndTemporary(&temp);
		fmemAlwaysAssert(temp.size == 0 && temp.used == 0);

		fmemAlwaysAssert(block.used == savedUsed);
		fmemAlwaysAssert(block.temporary == fmem_null);

		fmemFree(&block);
	}
}

static void TestFixed() {
	{
		fmemMemoryBlock block;
		fmemAlwaysAssert(!fmemInit(&block, fmemType_Fixed, 0, 0));
	}
	{
		fmemMemoryBlock block;
		fmemAlwaysAssert(fmemInit(&block, fmemType_Fixed, 1024, 0));
		fmemBlockHeader *hdr = FMEM__GETHEADER(&block);
		fmemAlwaysAssert((hdr->next == fmem_null) && (hdr->prev == fmem_null));
		fmemAlwaysAssert((block.size >= 1024) && (block.used == 0));

		uint8_t *mem1 = fmemPush(&block, 512, fmemPushFlags_None);
		fmemAlwaysAssert(fmemGetRemainingSize(&block) == 512);
		uint8_t *mem2 = fmemPush(&block, 512, fmemPushFlags_None);
		fmemAlwaysAssert(fmemGetRemainingSize(&block) == 0);
		uint8_t *mem3 = fmemPush(&block, 64, fmemPushFlags_None);
		fmemAlwaysAssert(mem3 == fmem_null);

		fmemFree(&block);
	}
}

static void TestGrowable(const bool withInit, const bool withAlloc) {
	fmemMemoryBlock block;
	if(withInit) {
		if(withAlloc) {
			fmemAlwaysAssert(fmemInit(&block, fmemType_Growable, 64, 0));
		} else {
			fmemInit(&block, fmemType_Growable, 0, 0);
		}
	} else {
		FMEM_MEMSET(&block, 0, sizeof(block));
	}

	// Initial block
	{
		uint8_t *data = fmemPush(&block, 1, fmemPushFlags_None);
		fmemBlockHeader *hdr = FMEM__GETHEADER(&block);
		fmemAlwaysAssert((hdr->next == fmem_null) && (hdr->prev == fmem_null));
		fmemAlwaysAssert((block.size > 1) && (block.used == 1));
		*data = 128;
	}
	{
		size_t dataSize = 2 * 24;
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_Clear);
		for(size_t i = 0; i < dataSize; ++i) {
			fmemAlwaysAssert(data[i] == 0);
		}
	}
	{
		size_t dataSize = fmemGetRemainingSize(&block);
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		fmemAlwaysAssert(block.used == block.size);
		fmemAlwaysAssert(fmemGetRemainingSize(&block) == 0);
	}

	// New block
	{
		size_t dataSize = FMEM_MEGABYTES(32);
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		fmemAlwaysAssert(data != fmem_null);
	}
	{
		size_t dataSize = 16;
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		fmemAlwaysAssert(data != fmem_null);
	}

	fmemFree(&block);
}

static void TestResetGrowable() {
	// Pushes that need three blocks, then a reset, four times over: every block has to be empty after a reset and no round may allocate a new one
	const size_t pushSize = 3000;
	const int pushCount = 3;
	const int roundCount = 4;
	fmemMemoryBlock block;
	fmemAlwaysAssert(fmemInit(&block, fmemType_Growable, 0, 0));
	size_t firstRoundTotalSize = 0;
	for(int round = 0; round < roundCount; ++round) {
		for(int pushIndex = 0; pushIndex < pushCount; ++pushIndex) {
			uint8_t *data = fmemPush(&block, pushSize, fmemPushFlags_None);
			fmemAlwaysAssert(data != fmem_null);
		}
		size_t totalSize = fmemGetTotalSize(&block);
		if(round == 0) {
			firstRoundTotalSize = totalSize;
		}
		fmemAlwaysAssert(totalSize == firstRoundTotalSize);
		fmemReset(&block);
		fmemAlwaysAssert(fmemGetRemainingSize(&block) == totalSize);
	}
	fmemFree(&block);
}

static void TestBlockSizeRounding() {
	// A minimum block size that is a multiple of a page stays as it is, and the block for a small push is exactly that size
	const size_t minBlockSize = FMEM_MEGABYTES(16);
	fmemMemoryBlock block;
	fmemAlwaysAssert(fmemInit(&block, fmemType_Growable, 0, minBlockSize));
	fmemAlwaysAssert(block.minBlockSize == minBlockSize);
	uint8_t *data = fmemPush(&block, 1, fmemPushFlags_None);
	fmemAlwaysAssert(data != fmem_null);
	fmemAlwaysAssert(block.size + FMEM__BLOCK_META_SIZE == minBlockSize);
	fmemFree(&block);
}

static void TestGrowMiddle() {
	fmemMemoryBlock mainBlock;
	fmemInit(&mainBlock, fmemType_Growable, 4096, 0);

	fmemPush(&mainBlock, 32 * 1024, fmemPushFlags_None);

	fmemFree(&mainBlock);
}

int main(int argc, char **args) {
	TestGrowable(false, false);
	TestGrowable(true, false);
	TestGrowable(true, true);
	TestFixed();
	TestTemporary();
	TestGrowMiddle();
	TestResetGrowable();
	TestBlockSizeRounding();
	return 0;
}