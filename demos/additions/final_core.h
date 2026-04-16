/*
Name:
	Final Core

Description:
	Core definitions for the final_framework, such as memory allocators, types, etc.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_CORE_H
#define FINAL_CORE_H

#include <final_platform_layer.h>

#define MEMORY_ALLOCATOR_ALLOC_FUNC(name) void *name(const size_t size, void *userData)
typedef MEMORY_ALLOCATOR_ALLOC_FUNC(MemoryAllcatorAllocFunc);

#define MEMORY_ALLOCATOR_FREE_FUNC(name) void name(void *ptr, void *userData)
typedef MEMORY_ALLOCATOR_FREE_FUNC(MemoryAllocatorFreeFunc);

typedef struct MemoryAllocator {
	MemoryAllcatorAllocFunc *allocate;
	MemoryAllocatorFreeFunc *free;
	void *userData;
	uintptr_t padding;
} MemoryAllocator;

fpl_internal void *DefaultMemoryAllocatorAlloc(const size_t size, void *userData) {
	if (size == 0) {
		return fpl_null;
	}
	void *result = fplMemoryAllocate(size);
	return(result);
}

fpl_internal void DefaultMemoryAllocatorFree(void *ptr, void *userData) {
	if (ptr == fpl_null) {
		return;
	}
	fplMemoryFree(ptr);
}

fpl_internal MemoryAllocator gDefaultMemoryAllocator = {
	DefaultMemoryAllocatorAlloc,
	DefaultMemoryAllocatorFree,
	fpl_null,
	0,
};

fpl_internal_inline void *MemoryAllocatorAlloc(const MemoryAllocator *allocator, const size_t size) {
	if (allocator == fpl_null) {
		allocator = &gDefaultMemoryAllocator;
	}
	fplAssertPtr(allocator->allocate);
	void *result = allocator->allocate(size, allocator->userData);
	return result;
}

fpl_internal_inline void MemoryAllocatorFree(const MemoryAllocator *allocator, void *ptr) {
	if (allocator == fpl_null) {
		allocator = &gDefaultMemoryAllocator;
	}
	fplAssertPtr(allocator->free);
	allocator->free(ptr, allocator->userData);
}

#endif // FINAL_CORE_H