#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <final_platform_layer.h>

#include <malloc.h>
#include <string.h>

uint32_t NextPowerOfTwoU32(uint32_t n) {
	--n;

	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;

	return n + 1;
}

//
// Fixed Array
//
typedef struct FixedArray {
	void *memory;
	size_t size;
	size_t count;
} FixedArray;

void FreeFixedArray(FixedArray *fixedArray) {
	if(fixedArray != fpl_null) {
		if(fixedArray->memory != fpl_null) {
			free(fixedArray->memory);
		}
		fplClearStruct(fixedArray);
	}
}

FixedArray AllocFixedArray(const size_t count, const size_t elementSize) {
	FixedArray result = fplZeroInit;
	result.count = count;
	result.size = count * elementSize;
	result.memory = malloc(result.size);
	memset(result.memory, 0, result.size);
	return(result);
}

#define FIXED_TYPED_ARRAY(type, name) \
struct { \
	FixedArray __arr; \
	type *items; \
	uint32_t itemCount; \
} ##name

#define ALLOC_FIXED_TYPED_ARRAY(ptr, type, count) \
{ \
	assert(ptr != fpl_null); \
	(ptr)->__arr = AllocFixedArray(count, sizeof(type)); \
	(ptr)->itemCount = count; \
	(ptr)->items = (type *)(ptr)->__arr.memory; \
}

#define FREE_FIXED_TYPED_ARRAY(ptr) \
{ \
	assert(ptr != fpl_null); \
	FreeFixedArray(&(ptr)->__arr); \
	(ptr)->items = fpl_null; \
	(ptr)->itemCount = 0; \
}

//
// Static Memory
//
#define MAX_STATIC_MEMORY_CHUNK_SIZE 4064UL // A bit less than one 4k page worth
typedef struct StaticMemoryChunk {
	uint8_t data[MAX_STATIC_MEMORY_CHUNK_SIZE];
	struct StaticMemoryChunk *next;
	void *base; // Need to remember the base pointer because we allocate more than one chunk at once, but we dont want to free it once
	uint32_t used;
} StaticMemoryChunk;
fplStaticAssert(sizeof(StaticMemoryChunk) <= 4096);

void FreeStaticMemoryChunks(StaticMemoryChunk *firstChunk) {
	void *lastBase = fpl_null;
	StaticMemoryChunk *chunk = firstChunk;
	while(chunk != fpl_null && chunk->base != lastBase) {
		void *base = lastBase = chunk->base;
	
		// Find next chunk with a different base
		StaticMemoryChunk *next = chunk->next;
		while(next != fpl_null) {
			if(next->base != lastBase) {
				break; // Found a new base, so we have a new next now
			}
			next = next->next;
		}

		fplMemoryFree(base);

		chunk = next;
	}
}

StaticMemoryChunk *AllocStaticMemoryChunks(const uint32_t count) {
	StaticMemoryChunk *result = (StaticMemoryChunk *)fplMemoryAllocate(sizeof(StaticMemoryChunk) * count);
	for(uint32_t i = 0; i < count; ++i) {
		result[i].next = (i < count - 1) ? &result[i + 1] : fpl_null;
		result[i].base = result;
	}
return(result);
}

typedef struct StringTable {
	const char **items; // Each item points to a memory offset inside a StaticMemoryChunk
	StaticMemoryChunk *__allChunks;
	StaticMemoryChunk *firstEmpty;
	StaticMemoryChunk *firstUsed;
	uint32_t count;
	uint32_t capacity;
} StringTable;

StringTable AllocStringTable(const uint32_t initialCapacity) {
	assert(initialCapacity > 0);
	StringTable result = fplZeroInit;
	result.capacity = NextPowerOfTwoU32(initialCapacity);
	result.items = (const char **)malloc(sizeof(const char *) * result.capacity);
	result.__allChunks = AllocStaticMemoryChunks(4);
	result.firstEmpty = result.__allChunks;
	return(result);
}

void FreeStringTable(StringTable *table) {
	if(table->__allChunks != fpl_null) {
		FreeStaticMemoryChunks(table->__allChunks);
	}
	free((void *)table->items);
	fplClearStruct(table);
}

int32_t PushStringToTable(StringTable *table, const char *sourceString) {
	if(table == fpl_null) return(-1);

	assert(table->capacity > 0);

	// First grow the items when too small
	if(table->count == table->capacity) {
		const char *oldFirst = table->items[0];
		uint32_t newCapacity = table->capacity * 2;
		const char **newItems = realloc((void *)table->items, sizeof(char *) * newCapacity);
		const char *newFirst = newItems[0];
		assert(newFirst == oldFirst);

		table->items = newItems;
		table->capacity = newCapacity;
	}
	
	assert(table->count < table->capacity);

	char *destString = fpl_null;
	bool sourceCopied = false;
	if(sourceString != fpl_null) {
		uint32_t requiredLen = (uint32_t)strlen(sourceString) + 1;
		assert(requiredLen <= MAX_STATIC_MEMORY_CHUNK_SIZE); // We dont allow more than 2048 bytes of contiguous memory

		StaticMemoryChunk *foundChunk = fpl_null;

		// Second find a used chunk which have enough memory left
		{
			StaticMemoryChunk *chunk = table->firstUsed;
			while(chunk != fpl_null) {
				StaticMemoryChunk *next = chunk->next;
				if((chunk->used + requiredLen) <= MAX_STATIC_MEMORY_CHUNK_SIZE) {
					foundChunk = chunk;
					break;
				}
				chunk = next;
			}
		}

		// Third find a free chunk
		if(foundChunk == fpl_null) {
			if(table->firstEmpty == fpl_null) {
				StaticMemoryChunk *newChunk = AllocStaticMemoryChunks(4);
				table->firstEmpty = newChunk;
			}
			assert(table->firstEmpty != fpl_null);
			StaticMemoryChunk *empty = table->firstEmpty;
			table->firstEmpty = empty->next;
			empty->next = table->firstUsed;
			table->firstUsed = empty;
			foundChunk = empty;
		}

		assert(foundChunk != fpl_null);

		destString = (uint8_t *)foundChunk->data + foundChunk->used;
		fplCopyString(sourceString, destString, requiredLen);
		foundChunk->used += requiredLen;
		sourceCopied = true;
	}

	int32_t result = table->count;
	table->items[result] = destString;
	++table->count;

	return(result);
}

#endif // CONTAINERS_H