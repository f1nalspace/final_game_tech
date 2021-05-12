#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <final_platform_layer.h>

#include <malloc.h>
#include <string.h>

// Resources:
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-1/
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-2/

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

#define FIXED_TYPED_ARRAY_INNER(type) \
	FixedArray __arr; \
	type *items; \
	uint32_t itemCount;

#define FIXED_TYPED_ARRAY(type, name) \
struct { \
	FIXED_TYPED_ARRAY_INNER(type) \
} ##name

#define ALLOC_FIXED_TYPED_ARRAY(ptr, type, count) \
{ \
	fplAssert((ptr) != fpl_null); \
	(ptr)->__arr = AllocFixedArray(count, sizeof(type)); \
	(ptr)->itemCount = count; \
	(ptr)->items = (type *)(ptr)->__arr.memory; \
}

#define FREE_FIXED_TYPED_ARRAY(ptr) \
{ \
	fplAssert((ptr) != fpl_null); \
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
	size_t used;
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

// Growable Array (Inspired by stretchy buffers (c) Sean Barret)
typedef struct GrowableArray {
	size_t count;
	size_t capacity;
	size_t elementSize;
	void *base;
} GrowableArray;

// Example int array
typedef struct IntGrowableArray {
	union {
		GrowableArray arr;
		struct {
			size_t count;
			size_t capacity;
			size_t elementSize;
			int *items;
		};
	};
} IntGrowableArray;

#define MIN_GROWABLE_ARRAY_CAPACITY 8

static void FreeGrowableArray(GrowableArray *arr) {
	fplAssert(arr != fpl_null);
	if(arr->base != fpl_null) {
		free(arr->base);
	}
	fplClearStruct(arr);
}

static void ResizeGrowableArray(GrowableArray *arr) {
	fplAssert(arr != fpl_null);
	fplAssert(arr->elementSize > 0);
	size_t newCapacity = fplMax(MIN_GROWABLE_ARRAY_CAPACITY, arr->capacity * 2);
	if(arr->base == fpl_null) {
		arr->capacity = newCapacity;
		arr->base = malloc(arr->elementSize * arr->capacity);
	} else {
		void *oldBase = arr->base;
		arr->capacity = newCapacity;
		arr->base = realloc(oldBase, arr->elementSize * arr->capacity);
	}
}

#define INIT_GROWABLE_ARRAY(arr, type) \
	(arr)->elementSize = sizeof(type);


static size_t IncGrowableArray(GrowableArray *arr) {
	if(arr->count == arr->capacity) {
		ResizeGrowableArray(arr);
	}
	fplAssert(arr->count < arr->capacity);
	size_t result = arr->count;
	++arr->count;
	return(result);
}

#define PUSH_TO_GROWABLE_ARRAY(ptr, value) \
{ \
	size_t index = IncGrowableArray(&(ptr)->arr); \
	(ptr)->items[index] = value; \
}

//
// Static Memory Pool, (Max of 4kb of one contiguous block of memory)
//
typedef struct StaticMemoryPool {
	StaticMemoryChunk *base;  // First base chunk
	StaticMemoryChunk *empty; // First empty chunk
	StaticMemoryChunk *used;  // First used chunk
} StaticMemoryPool;

static StaticMemoryPool AllocStaticMemoryPool(const uint32_t initialChunkCount) {
	StaticMemoryPool result = fplZeroInit;
	result.base = AllocStaticMemoryChunks(initialChunkCount);
	result.empty = result.base;
	return(result);
}

static void FreeStaticMemoryPool(StaticMemoryPool *pool) {
	if(pool == fpl_null) return;
	if(pool->base == fpl_null) return;
	FreeStaticMemoryChunks(pool->base);
	fplClearStruct(pool);
}

static StaticMemoryChunk *GetAvailableStaticMemoryChunk(StaticMemoryPool *pool, const size_t size) {
	StaticMemoryChunk *foundChunk = fpl_null;

	// Second find a used chunk which have enough memory left
	{
		StaticMemoryChunk *chunk = pool->used;
		while(chunk != fpl_null) {
			StaticMemoryChunk *next = chunk->next;
			if((chunk->used + size) <= MAX_STATIC_MEMORY_CHUNK_SIZE) {
				foundChunk = chunk;
				break;
			}
			chunk = next;
		}
	}

	// Third find a free chunk
	if(foundChunk == fpl_null) {
		if(pool->empty == fpl_null) {
			StaticMemoryChunk *newChunk = AllocStaticMemoryChunks(4);
			pool->empty = newChunk;
		}
		fplAssert(pool->empty != fpl_null);
		StaticMemoryChunk *empty = pool->empty;
		pool->empty = empty->next;
		empty->next = pool->used;
		pool->used = empty;
		foundChunk = empty;
	}

	return(foundChunk);
}

typedef struct StringTable {
	union {
		GrowableArray arr;
		struct {
			size_t count;
			size_t capacity;
			size_t elementSize;
			const char **items;
		};
	};
	StaticMemoryPool pool;
} StringTable;

StringTable AllocStringTable() {
	StringTable result = fplZeroInit;
	result.pool = AllocStaticMemoryPool(4);
	INIT_GROWABLE_ARRAY(&result.arr, const char *);
	return(result);
}

void FreeStringTable(StringTable *table) {
	FreeStaticMemoryPool(&table->pool);
	FreeGrowableArray(&table->arr);
	fplClearStruct(table);
}

size_t PushStringToTable(StringTable *table, const char *sourceString) {
	if(table == fpl_null) return(UINT32_MAX);

	char *destString = fpl_null;
	bool sourceCopied = false;
	if(sourceString != fpl_null) {
		size_t requiredLen = strlen(sourceString) + 1;
		fplAssert(requiredLen <= MAX_STATIC_MEMORY_CHUNK_SIZE); // We dont allow more than 2048 bytes of contiguous memory

		StaticMemoryChunk *foundChunk = GetAvailableStaticMemoryChunk(&table->pool, requiredLen);
		fplAssert(foundChunk != fpl_null);

		destString = (uint8_t *)foundChunk->data + foundChunk->used;
		fplCopyString(sourceString, destString, requiredLen);
		foundChunk->used += requiredLen;
		sourceCopied = true;
	}
	size_t index = table->count;
	PUSH_TO_GROWABLE_ARRAY(table, destString);
	return(index);
}

#endif // CONTAINERS_H