/*
Name:
	Final Gamebox

	Frontend-Part (Utils Implementation)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#include "utils.h"

extern String StringCreateFromSource(fmemMemoryBlock *memory, const char *source) {
	size_t length = fplGetStringLength(source);
	char *text = fmemPush(memory, length + 1, fmemPushFlags_Clear);
	fplCopyStringLen(source, length, text, length + 1);
	String result = { 0 };
	result.len = length;
	result.text = text;
	return result;
}

#define MIN_STRING_TABLE_ENTRY_COUNT 16
#define STRING_TABLE_MEMORY_SPACING 4

extern StringList StringListInit(fmemMemoryBlock *memory) {
	StringList result = fplZeroInit;
	result.memory = memory;
	return result;
}

extern void StringListClear(StringList *list) {
	list->capacity = list->count = 0;
	list->entries = fpl_null;
	fmemFreeChildren(list->memory);
}

extern size_t StringListAdd(StringList *list, const char *source) {
	size_t length = fplGetStringLength(source);

	char *text = fmemPush(list->memory, length + 1 + STRING_TABLE_MEMORY_SPACING, fmemPushFlags_Clear);

	fplCopyStringLen(source, length, text, length + 1);

	if (list->capacity == 0) {
		list->capacity = MIN_STRING_TABLE_ENTRY_COUNT;
		list->count = 0;
		list->entries = (String *)fmemPush(list->memory, sizeof(String) * list->capacity, fmemPushFlags_Clear);
	} else if (list->capacity == list->count) {
		list->capacity *= 2;
		String *oldEntries = list->entries;
		String *newEntries = (String *)fmemPush(list->memory, sizeof(String) * list->capacity, fmemPushFlags_Clear);
		if (newEntries == fpl_null) {
			return SIZE_MAX;
		}
		for (size_t i = 0; i < list->count; ++i) {
			newEntries[i] = oldEntries[i];
		}
		list->entries = newEntries;
	}

	if (list->entries == fpl_null) {
		return SIZE_MAX;
	}

	String str = fplZeroInit;
	str.len = length;
	str.text = text;

	size_t result = list->count;

	list->entries[list->count] = str;
	list->count++;

	return result;
}

extern IndexHashtable IndexHashtableInit(fmemMemoryBlock *memory) {
	IndexHashtable result = { 0 };
	result.memory = memory;
	return result;
}

extern void IndexHashtableClear(IndexHashtable *hashTable) {
	fmemReset(hashTable->memory);
	hashTable->bucketCount = 0;
	hashTable->firstBucket = hashTable->lastBucket = NULL;
}

extern bool IndexHashtableAdd(IndexHashtable *hashTable, const uint64_t key, const uint64_t index) {
	uint64_t bucketNum = (key / INDEX_HASHTABLE_ENTRIES_PER_BUCKET) + 1;
	FGB_ASSERT(bucketNum <= SIZE_MAX);

	IndexHashtableBucket *foundBucket = NULL;

	if (hashTable->bucketCount < bucketNum) {
		// Create required buckets
		size_t missingBucketCount = (size_t)bucketNum - hashTable->bucketCount;
		for (size_t bucketIndex = 0; bucketIndex < missingBucketCount; ++bucketIndex) {

			IndexHashtableBucket *bucket = (IndexHashtableBucket *)fmemPush(hashTable->memory, sizeof(IndexHashtableBucket), fmemPushFlags_Clear);
			if (bucket == NULL) {
				return false;
			}
			fplClearStruct(bucket->entries);
			bucket->next = NULL;

			if (hashTable->lastBucket == NULL) {
				hashTable->firstBucket = hashTable->lastBucket = bucket;
			} else {
				hashTable->lastBucket->next = (struct IndexHashtableBucket *)bucket;
				hashTable->lastBucket = bucket;
			}
			foundBucket = bucket;
		}
		hashTable->bucketCount += missingBucketCount;
	} else {
		IndexHashtableBucket *bucket = hashTable->firstBucket;
		size_t bucketIndex = 0;
		while (bucket != NULL) {
			if (bucketIndex == (bucketNum - 1)) {
				foundBucket = bucket;
				break;
			}
			IndexHashtableBucket *next = (IndexHashtableBucket *)bucket->next;
			bucket = next;
			++bucketIndex;
		}
	}

	fplAssert(foundBucket != NULL);

	uint64_t bucketIndex = bucketNum - 1;
	fplAssert(bucketIndex < hashTable->bucketCount);

	uint64_t entryIndex = (key - bucketIndex * INDEX_HASHTABLE_ENTRIES_PER_BUCKET);
	fplAssert(entryIndex < INDEX_HASHTABLE_ENTRIES_PER_BUCKET);

	if (foundBucket->entries[entryIndex].occupied) {
		return false;
	}

	foundBucket->entries[entryIndex].key = key;
	foundBucket->entries[entryIndex].index = index;
	foundBucket->entries[entryIndex].occupied = 1;

	return true;
}

extern bool IndexHashtableGet(const IndexHashtable *hashTable, const uint64_t key, size_t *outIndex) {
	uint64_t bucketNum = (key / INDEX_HASHTABLE_ENTRIES_PER_BUCKET) + 1;

	uint64_t bucketIndex = bucketNum - 1;
	if (bucketIndex >= hashTable->bucketCount) {
		return false;
	}

	IndexHashtableBucket *foundBucket = NULL;

	IndexHashtableBucket *bucket = hashTable->firstBucket;
	size_t currentIndex = 0;
	while (bucket != NULL) {
		if (currentIndex == bucketIndex) {
			foundBucket = bucket;
			break;
		}
		IndexHashtableBucket *next = (IndexHashtableBucket *)bucket->next;
		bucket = next;
		++currentIndex;
	}

	if (foundBucket == NULL) {
		return false;
	}

	uint64_t entryIndex = (key - bucketIndex * INDEX_HASHTABLE_ENTRIES_PER_BUCKET);
	fplAssert(entryIndex < INDEX_HASHTABLE_ENTRIES_PER_BUCKET);

	if (!foundBucket->entries[entryIndex].occupied || foundBucket->entries[entryIndex].key != key) {
		return false;
	}

	*outIndex = (size_t)foundBucket->entries[entryIndex].index;
	return true;
}


static uint32_t NextPowerOfTwo(const uint32_t input) {
	uint32_t x = input;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return(x);
}
static uint32_t PrevPowerOfTwo(const uint32_t input) {
	uint32_t result = NextPowerOfTwo(input) >> 1;
	return(result);
}

extern uint32_t RoundToPowerOfTwo(const uint32_t input) {
	uint32_t prev = PrevPowerOfTwo(input);
	uint32_t next = NextPowerOfTwo(input);
	if (prev == input)
		return prev;
	else if (next == input)
		return next;
	else if ((next - input) < (input - prev))
		return prev;
	else
		return next;
}