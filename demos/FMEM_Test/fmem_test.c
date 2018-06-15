#define FMEM_IMPLEMENTATION
#include <final_memory.h>

static void TestTemporary() {
	{
		uint8_t *data;

		fmemMemoryBlock block;
		FMEM_ASSERT(fmemInit(&block, fmemType_Growable, 1024));

		data = fmemPush(&block, 32, fmemPushFlags_None);
		size_t savedUsed = block.used;

		fmemMemoryBlock temp;
		FMEM_ASSERT(fmemBeginTemporary(&block, &temp));
		FMEM_ASSERT(block.temporary == &temp);
		FMEM_ASSERT(block.size == block.used);
		data = fmemPush(&temp, 256, fmemPushFlags_Clear);
		FMEM_ASSERT(data != fmem_null);

		FMEM_ASSERT(fmemPush(&block, 64, fmemPushFlags_None) == fmem_null);

		data = fmemPush(&temp, 1024, fmemPushFlags_None);
		FMEM_ASSERT(data == fmem_null);

		size_t remaining = fmemGetRemainingSize(&temp);
		data = fmemPush(&temp, remaining, fmemPushFlags_None);
		FMEM_ASSERT(temp.used == temp.size);

		fmemEndTemporary(&temp);
		FMEM_ASSERT(temp.size == 0 && temp.used == 0);

		FMEM_ASSERT(block.used == savedUsed);
		FMEM_ASSERT(block.temporary == fmem_null);

		fmemFree(&block);
	}
}

static void TestFixed() {
	{
		fmemMemoryBlock block;
		FMEM_ASSERT(!fmemInit(&block, fmemType_Fixed, 0));
	}
	{
		fmemMemoryBlock block;
		FMEM_ASSERT(fmemInit(&block, fmemType_Fixed, 1024));
		fmemBlockHeader *hdr = FMEM__GETHEADER(&block);
		FMEM_ASSERT((hdr->next == fmem_null) && (hdr->prev == fmem_null));
		FMEM_ASSERT((block.size >= 1024) && (block.used == 0));

		uint8_t *mem1 = fmemPush(&block, 512, fmemPushFlags_None);
		FMEM_ASSERT(fmemGetRemainingSize(&block) == 512);
		uint8_t *mem2 = fmemPush(&block, 512, fmemPushFlags_None);
		FMEM_ASSERT(fmemGetRemainingSize(&block) == 0);
		uint8_t *mem3 = fmemPush(&block, 64, fmemPushFlags_None);
		FMEM_ASSERT(mem3 == fmem_null);

		fmemFree(&block);
	}
}

static void TestGrowable(const bool withInit, const bool withAlloc) {
	fmemMemoryBlock block;
	if(withInit) {
		if(withAlloc) {
			FMEM_ASSERT(fmemInit(&block, fmemType_Growable, 64));
		} else {
			fmemInit(&block, fmemType_Growable, 0);
		}
	} else {
		FMEM_MEMSET(&block, 0, sizeof(block));
	}

	// Initial block
	{
		uint8_t *data = fmemPush(&block, 1, fmemPushFlags_None);
		fmemBlockHeader *hdr = FMEM__GETHEADER(&block);
		FMEM_ASSERT((hdr->next == fmem_null) && (hdr->prev == fmem_null));
		FMEM_ASSERT((block.size > 1) && (block.used == 1));
		*data = 128;
	}
	{
		size_t dataSize = 2 * 24;
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_Clear);
		for(size_t i = 0; i < dataSize; ++i) {
			FMEM_ASSERT(data[i] == 0);
		}
	}
	{
		size_t dataSize = fmemGetRemainingSize(&block);
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		FMEM_ASSERT(block.used == block.size);
		FMEM_ASSERT(fmemGetRemainingSize(&block) == 0);
	}

	// New block
	{
		size_t dataSize = FMEM_MEGABYTES(32);
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		FMEM_ASSERT(data != fmem_null);
	}
	{
		size_t dataSize = 16;
		uint8_t *data = fmemPush(&block, dataSize, fmemPushFlags_None);
		FMEM_ASSERT(data != fmem_null);
	}

	fmemFree(&block);
}

int main(int argc, char **args) {
	TestGrowable(false, false);
	TestGrowable(true, false);
	TestGrowable(true, true);
	TestFixed();
	TestTemporary();
	return 0;
}