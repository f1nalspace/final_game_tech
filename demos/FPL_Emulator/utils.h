/*
Name:
	Final Gamebox

	Frontend-Part (Utils Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#pragma once

#include <final_platform_layer.h>

#include <final_memory.h>

typedef struct {
	uint64_t key;
	uint64_t index;
	uint64_t occupied;
	uint64_t padding;
} IndexHashtableEntry;

#define INDEX_HASHTABLE_ENTRIES_PER_BUCKET 2048ULL

typedef struct {
	IndexHashtableEntry entries[INDEX_HASHTABLE_ENTRIES_PER_BUCKET];
	struct IndexHashtableBucket *next;
} IndexHashtableBucket;

typedef struct {
	fmemMemoryBlock *memory;
	IndexHashtableBucket *firstBucket;
	IndexHashtableBucket *lastBucket;
	size_t bucketCount;
} IndexHashtable;

extern IndexHashtable IndexHashtableInit(fmemMemoryBlock *memory);
extern void IndexHashtableClear(IndexHashtable *hashTable);
extern bool IndexHashtableAdd(IndexHashtable *hashTable, const uint64_t key, const uint64_t index);
extern bool IndexHashtableGet(const IndexHashtable *hashTable, const uint64_t key, size_t *outIndex);

typedef struct {
	char *text;
	size_t len;
} String;

static String EmptyString = fplZeroInit;

typedef struct {
	fmemMemoryBlock *memory;
	String *entries;
	size_t count;
	size_t capacity;
} StringList;

extern StringList StringListInit(fmemMemoryBlock *memory);
extern size_t StringListAdd(StringList *list, const char *source);
extern void StringListClear(StringList *list);

extern String StringCreateFromSource(fmemMemoryBlock *memory, const char *source);

extern uint32_t RoundToPowerOfTwo(const uint32_t input);

extern bool StringCompareIgnoreCase(const char *a, const char *b);