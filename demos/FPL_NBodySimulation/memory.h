#ifndef MEMORY_H
#define MEMORY_H

#include <final_platform_layer.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

struct MemoryBlock {
	size_t size;
	size_t offset;
	void *base;
};

inline MemoryBlock AllocateMemoryBlock(const size_t size) {
	MemoryBlock result = {};
	result.size = size;
	result.base = fplMemoryAllocate(size);
	return(result);
}

inline void ReleaseMemoryBlock(MemoryBlock *block) {
	if (block->base != nullptr) {
		fplMemoryFree(block->base);
	}
	*block = {};
}

template <typename T>
inline T *PushSize(MemoryBlock *block, const size_t size, const bool clear = true) {
	assert((block->offset + size) <= block->size);
	void *ptr = (void *)((uint8_t *)block->base + block->offset);
	block->offset += size;
	if (clear) {
		fplMemoryClear(ptr, size);
	}
	T *result = (T*)ptr;
	return(result);
}

template <typename T>
inline T *PushStruct(MemoryBlock *block, const bool clear = true) {
	T *result = PushSize<T>(block, sizeof(T), clear);
	return(result);
}

template <typename T>
inline T *PushArray(MemoryBlock *block, const size_t count, const bool clear = true) {
	T *result = PushSize<T>(block, count * sizeof(T), clear);
	return(result);
}

inline void PopSize(MemoryBlock *block, const size_t size) {
	assert((block->offset - size) >= 0);
	block->offset -= size;
}

#endif
