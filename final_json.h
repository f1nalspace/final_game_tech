/***
final_json.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file JSON reader/writer C99 library.

This library is designed to parse or generate a UTF-8 json-byte stream.

It uses a block allocator memory scheme based on malloc.
Use FJSON_MALLOC/FJSON_FREE to provide your own memory allocation function.

The only dependencies are a C99 complaint compiler.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you want.
- Define FJSON_IMPLEMENTATION before including this header file in your main translation unit.

-------------------------------------------------------------------------------
	Usage
-------------------------------------------------------------------------------

#define FJSON_IMPLEMENTATION
#include <final_json.h>

...

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final JSON is released under the following license:

MIT License

Copyright (c) 2024 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_json.h
	\version v0.1.0 alpha
	\author Torsten Spaete
	\brief Final JSON (FJSON) - A open source C99 single file header json reader/writer library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.1.0 alpha:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- Encoding support
		- ISO8859-1
		- ASCII
		- UTF-8
		- UTF-16 / Unicode

*/

#ifndef FJSON_H
#define FJSON_H

// Detect compiler
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
#	define FJSON_IS_C99
#elif defined(__cplusplus)
#	define FJSON_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

// Api export
#if defined(FJSON_PRIVATE)
#	define fjson_api static
#else
#	define fjson_api extern
#endif

// Malloc functions override
#ifndef FJSON_MALLOC
#	include <malloc.h>
#	define FJSON_MALLOC(size) malloc(size)
#	define FJSON_REALLOC(ptr, size) realloc(ptr, size)
#	define FJSON_FREE(ptr) free(ptr)
#endif
#ifndef FJSON_STRING
#	include <string.h>
#	define FJSON_MEMSET(dst, value, size) memset(dst, value, size)
#	define FJSON_STRNCPY(dst, dstCount, src, srcCount) strncpy_s(dst, srcCount, src, dstCount)
#	define FJSON_STRLEN(str) strlen(str)
#endif
#ifndef FJSON_ASSERT
#	include <assert.h>
#	define FJSON_ASSERT(exp) assert(exp)
#endif

// Useful macros
#if defined(FJSON_IS_C99)
#	define FJSON_ZERO_INIT {0}
#else
#	define FJSON_ZERO_INIT {}
#endif
#define FJSON_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// Includes
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define fjson_null NULL

// CPP Begin/End
#if defined(FJSON_IS_CPP)
#define FJSON_CPP_BEGIN extern "C" {
#define FJSON_CPP_END }
#else
#define FJSON_CPP_BEGIN
#define FJSON_CPP_END
#endif

//
// Header / API
//
FJSON_CPP_BEGIN

typedef enum fjsonType {
	fjsonType_None = 0,
	fjsonType_Object,
	fjsonType_Array,
	fjsonType_Integer,
	fjsonType_Float,
	fjsonType_Bool,
	fjsonType_String,
	fjsonType_Null,
} fjsonType;

typedef struct fjsonString {
	const char *text;
	size_t length;
} fjsonString;

typedef struct jsonInteger {
	int64_t value;
} fjsonInteger;

typedef struct fjsonFloat {
	double value;
} fjsonFloat;

typedef struct fjsonBool {
	int value;
} fjsonBool;

typedef uint32_t fjsonNull;

typedef struct fjsonElement {
	fjsonType type;
	union {
		const struct fjsonObject *objectValue;
		const struct fjsonArray *arrayValue;
		fjsonFloat floatValue;
		fjsonInteger integerValue;
		fjsonString stringValue;
		fjsonBool boolValue;
		fjsonNull nullValue;
	};
} fjsonElement;

typedef struct fjsonObjectItem {
	const char *name;
	fjsonElement *element;
	fjsonObjectItem *prev;
	fjsonObjectItem *next;
} fjsonObjectItem;

typedef struct fjsonObject {
	const char *name;
	fjsonObjectItem *first;
	fjsonObjectItem *last;
} fjsonObject;

typedef struct fjsonArray {
	const fjsonElement **items;
	size_t capacity;
	size_t count;
} fjsonArray;

typedef struct fjsonContext {
	fjsonElement *root;
} fjsonContext;

fjson_api fjsonContext *fjsonCreateContext();
fjson_api void fjsonFreeContext(fjsonContext *ctx);

fjson_api fjsonElement fjsonElementString(fjsonContext *ctx, const char *stringValue);
fjson_api fjsonElement fjsonElementFloat(fjsonContext *ctx, const double floatValue);
fjson_api fjsonElement fjsonElementInteger(fjsonContext *ctx, const int64_t intValue);
fjson_api fjsonElement fjsonElementBool(fjsonContext *ctx, const bool boolValue);
fjson_api fjsonElement fjsonElementObject(fjsonContext *ctx, const fjsonObject *obj);
fjson_api fjsonElement fjsonElementArray(fjsonContext *ctx, const fjsonArray *arr);
fjson_api fjsonElement fjsonElementNull(fjsonContext *ctx);

fjson_api fjsonObject *fjsonObjectCreate(fjsonContext *ctx, const char *name);

fjson_api bool fjsonObjectInsertElement(fjsonContext *ctx, fjsonObject *root, const char *name, const fjsonElement *element);
fjson_api bool fjsonObjectRemoveByName(fjsonContext *ctx, fjsonObject *root, const char *name);
fjson_api bool fjsonObjectClear(fjsonContext *ctx, fjsonObject *root);

fjson_api fjsonElement *fjsonObjectFindByName(fjsonObject *root, const char *name);
fjson_api bool fjsonObjectContainsName(fjsonObject *root, const char *name);

fjson_api fjsonArray *fjsonArrayCreate(fjsonContext *ctx);
fjson_api size_t fjsonArrayAppendChild(fjsonContext *ctx, fjsonArray *arr, const fjsonElement *child);
fjson_api bool fjsonArrayRemoveChild(fjsonContext *ctx, fjsonArray *arr, const fjsonElement *child);
fjson_api bool fjsonArrayClear(fjsonContext *ctx, fjsonArray *arr);

FJSON_CPP_END

#endif // FJSON_H

#if defined(FJSON_IMPLEMENTATION) && !defined(FJSON_IMPLEMENTED)
#define FJSON_IMPLEMENTED

FJSON_CPP_BEGIN

typedef struct fjson__MemoryBlock {
	uint8_t *data;
	fjson__MemoryBlock *nextBlock;
	size_t size;
	size_t used;
} fjson__MemoryBlock;

typedef struct {
	fjson__MemoryBlock *first;
	fjson__MemoryBlock *last;
} fjson__MemoryBlockPool;

typedef struct fjson__InternalContext {
	fjsonContext base;
	fjson__MemoryBlockPool objectPool;
	fjson__MemoryBlockPool stringPool;
	fjson__MemoryBlockPool genericPool;
} fjson__InternalContext;

#define FJSON__MEMORY_BLOCK_PADDING 32

#define FJSON__STRING_PADDING 8

#define FJSON__MIN_OBJECT_POOL_COUNT 64
#define FJSON__MIN_STRING_POOL_SIZE 1024
#define FJSON__MIN_ARRAY_POOL_COUNT 16

#define FJSON__MIN_GENERIC_POOL_SIZE 4096

#define FJSON__MEMORY_BLOCK_DATA_OFFSET sizeof(fjson__MemoryBlock) + FJSON__MEMORY_BLOCK_PADDING

#define FJSON__ALIGNMENT_OFFSET(value, alignment) ( (((alignment) > 1) && (((value) & ((alignment) - 1)) != 0)) ? ((alignment) - ((value) & (alignment - 1))) : 0)			
#define FJSON__ALIGNED_SIZE(size, alignment) (((size) > 0 && (alignment) > 0) ? ((size) + FJSON__ALIGNMENT_OFFSET(size, alignment)) : (size))

#if defined(FJSON_IS_C99)
#	define fjson__ZeroInit {0}
#else
#	define fjson__ZeroInit {}
#endif

#define fjson__ClearStruct(ptr) FJSON_MEMSET((void *)(ptr), 0, sizeof(*(ptr)))

#define FJSON__MAX(a, b) ((a) > (b) ? (a) : (b))

static void fjson__MemorySet(void *dst, const int value, const size_t size) {
	FJSON_MEMSET(dst, value, size);
}
static size_t fjson__GetStringLength(const char *str) {
	return FJSON_STRLEN(str);
}
static void fjson__StringCopyLen(const char *source, const size_t sourceLength, char *dest, const size_t destLength) {
	FJSON_STRNCPY(dest, destLength, source, sourceLength);
}

static inline bool fjson__IsAlpha(const char c) {
	bool result = (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
	return(result);
}
static inline bool fjson__IsNumeric(const char c) {
	bool result = (c >= 48 && c <= 57);
	return(result);
}
static inline bool fjson__IsAlphaNumeric(const char c) {
	bool result = fxml__IsAlpha(c) || fxml__IsNumeric(c);
	return(result);
}
static inline bool fjson__IsWhitespace(const char c) {
	bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r';
	return(result);
}

static bool fjson__IsEqualString(const char *a, const char *b) {
	if ((a == fjson_null) && (b == fjson_null)) {
		return true;
	}
	if (a == fjson_null || b == fjson_null) {
		return false;
	}
	while (true) {
		if (!*a && !*b) {
			break;
		}
		if ((!*a && *b) || (*a && !*b) || (*a != *b)) {
			return false;
		}
		++a;
		++b;
	}
	return(true);
}

static void *fjson__Alloc(const size_t size) {
	void *result = FJSON_MALLOC(size);
	if (result != fjson_null) {
		fjson__MemorySet(result, 0, size);
	}
	return result;
}

static void fjson__Free(void *base) {
	if (base != fjson_null) {
		FJSON_FREE(base);
	}
}

static fjson__MemoryBlock *fjson__AllocMemoryBlock(const size_t dataSize) {
	size_t totalSize = sizeof(fjson__MemoryBlock) + FJSON__MEMORY_BLOCK_PADDING + dataSize;

	uint8_t *mem = (uint8_t *)fjson__Alloc(totalSize);
	if (mem == fjson_null) {
		return fjson_null;
	}

	uint8_t *data = mem + FJSON__MEMORY_BLOCK_DATA_OFFSET;

	fjson__MemoryBlock *block = (fjson__MemoryBlock *)mem;
	block->size = dataSize;
	block->data = data;
	block->used = 0;
	block->nextBlock = fjson_null;
	return block;
}

static void fjson_FreeMemoryBlocks(fjson__MemoryBlock *root) {
	fjson__MemoryBlock *cur = root;
	while (cur != fjson_null) {
		fjson__Free(cur);
		cur = cur->nextBlock;
	}
}

static uint8_t *fjson__AquireMemory(fjson__MemoryBlockPool *pool, const size_t size, const size_t initialSize) {
	fjson__MemoryBlock *block;
	if ((pool->last == fjson_null) || (pool->last->used + size > pool->last->size)) {
		block = fjson__AllocMemoryBlock(initialSize);
		if (block == fjson_null) {
			return fjson_null;
		}
		if (pool->last != fjson_null) {
			pool->last->nextBlock = block;
			pool->last = block;
		} else {
			pool->last = pool->first = block;
		}
	} else {
		block = pool->last;
	}
	FJSON_ASSERT(block->used + size <= block->size);
	uint8_t *item = block->data + block->used;
	block->used += size;
	return item;
}

static char *fjson__CreateString(fjson__MemoryBlockPool *pool, const char *text, const size_t length) {
	size_t itemSize = length + 1 + FJSON__STRING_PADDING;

	size_t poolSize = itemSize;
	if (poolSize < FJSON__MIN_STRING_POOL_SIZE) {
		poolSize = FJSON__MIN_STRING_POOL_SIZE;
	} else {
		poolSize = FJSON__ALIGNED_SIZE(itemSize, 64);
	}

	char *result = (char *)fjson__AquireMemory(pool, itemSize, poolSize);

	if (result == fjson_null) {
		return fjson_null;
	}

	fjson__StringCopyLen(text, length, result, length + 1);

	return result;
}

fjson_api fjsonContext *fjsonCreateContext() {
	fjson__InternalContext *result = (fjson__InternalContext *)fjson__Alloc(sizeof(fjson__InternalContext));
	if (result == fjson_null) {
		return fjson_null;
	}
	return &result->base;
}

fjson_api void fjsonFreeContext(fjsonContext *ctx) {
	if (ctx == fjson_null) {
		return;
	}
	fjson__InternalContext *internalCtx = (fjson__InternalContext *)ctx;
	fjson_FreeMemoryBlocks(internalCtx->genericPool.first);
	fjson_FreeMemoryBlocks(internalCtx->stringPool.first);
	fjson_FreeMemoryBlocks(internalCtx->objectPool.first);
	fjson__ClearStruct(internalCtx);
}

fjson_api fjsonObject *fjsonObjectCreate(fjsonContext *ctx, const char *name) {
	if (ctx == fjson_null) {
		return fjson_null;
	}
	fjson__InternalContext *internalCtx = (fjson__InternalContext *)ctx;

	size_t itemSize = FJSON__MAX(sizeof(fjsonObject), sizeof(fjsonObjectItem));

	fjsonObject *result = (fjsonObject *)fjson__AquireMemory(&internalCtx->objectPool, itemSize, itemSize * FJSON__MIN_OBJECT_POOL_COUNT);
	if (result == fjson_null) {
		return fjson_null;
	}

	size_t nameLength = fjson__GetStringLength(name);

	result->name = fjson__CreateString(&internalCtx->stringPool, name, nameLength);

	return result;
}

static fjsonObjectItem *fjson__FindObjectByName(fjsonObject *root, const char *name) {
	fjsonObjectItem *cur = root->first;
	while (cur != fjson_null) {
		if (fjson__IsEqualString(cur->name, name)) {
			return cur;
		}
		cur = cur->next;
	}
	return fjson_null;
}

fjson_api bool fjsonObjectContainsName(fjsonObject *root, const char *name) {
	if (root == fjson_null || name == fjson_null) {
		return false;
	}
	fjsonObjectItem *item = fjson__FindObjectByName(root, name);
	return item != fjson_null;
}

fjson_api bool fjsonObjectInsertElement(fjsonContext *ctx, fjsonObject *root, const char *name, const fjsonElement *element) {
	if (ctx == fjson_null || root == fjson_null || name == fjson_null || element == fjson_null) {
		return false;
	}

	fjson__InternalContext *internalCtx = (fjson__InternalContext *)ctx;

	size_t nameLength = fjson__GetStringLength(name);
	if (nameLength == 0) {
		return false;
	}

	fjsonObjectItem *item = fjson__FindObjectByName(root, name);
	if (item != fjson_null) {
		return false;
	}


	size_t itemSize = FJSON__MAX(sizeof(fjsonObject), sizeof(fjsonObjectItem));

	item = (fjsonObjectItem *)fjson__AquireMemory(&internalCtx->objectPool, itemSize, itemSize * FJSON__MIN_OBJECT_POOL_COUNT);
	if (item == fjson_null) {
		return false;
	}

	const char *clonedName = fjson__CreateString(&internalCtx->stringPool, name, nameLength);
	if (clonedName == fjson_null) {
		return false;
	}

	item->name = clonedName;

	if (root->last != fjson_null) {
		item->prev = root->last;
		root->last->next = item;
	} else {
		root->last = root->first = item;
	}

	return true;
}

fjson_api bool fjsonObjectRemoveByName(fjsonContext *ctx, fjsonObject *root, const char *name) {
	if (ctx == fjson_null || root == fjson_null || name == fjson_null) {
		return false;
	}

	fjsonObjectItem *item = fjson__FindObjectByName(root, name);
	if (item == fjson_null) {
		return false;
	}

	fjsonObjectItem *next = item->next;
	fjsonObjectItem *prev = item->prev;

	if (item->prev == fjson_null) {
		// Head
		root->first = item->next;
		if (item->next == fjson_null) {
			root->first = fjson_null;
			root->last = fjson_null;
		} else {
			item->next->prev = fjson_null;
		}
	} else if (item->next == fjson_null) {
		// Tail
		root->last = item->prev;
		if (item->prev == fjson_null) {
			root->first = item->prev;
		} else {
			item->prev->next = fjson_null;
		}
	} else {
		// In between
		item->prev->next = item->next;
		item->next->prev = item->prev;
	}

	return true;
}

fjson_api bool fjsonObjectClear(fjsonContext *ctx, fjsonObject *root) {
	if (ctx == fjson_null || root == fjson_null) {
		return false;
	}

	fjsonObjectItem *cur = root->first;

	while (cur != fjson_null) {
		cur = cur->next;
	}

	return true;
}

fjson_api fjsonElement *fjsonObjectFindByName(fjsonObject *root, const char *name) {
	if (root == fjson_null || name == fjson_null) {
		return fjson_null;
	}

	fjsonObjectItem *item = fjson__FindObjectByName(root, name);
	if (item == fjson_null) {
		return fjson_null;
	}

	fjsonElement *result = item->element;
	return result;
}


fjson_api fjsonArray *fjsonArrayCreate(fjsonContext *ctx) {
	if (ctx == fjson_null) {
		return fjson_null;
	}
	fjson__InternalContext *internalCtx = (fjson__InternalContext *)ctx;

	fjsonArray *result = (fjsonArray *)fjson__AquireMemory(&internalCtx->genericPool, sizeof(fjsonArray), sizeof(fjsonArray) * FJSON__MIN_ARRAY_POOL_COUNT);
	if (result == fjson_null) {
		return fjson_null;
	}	

	return result;
}

fjson_api size_t fjsonArrayAppendChild(fjsonContext *ctx, fjsonArray *arr, const fjsonElement *child) {
	if (ctx == fjson_null || arr == fjson_null || child == fjson_null) {
		return 0;
	}

	if ((arr->capacity == 0) || ((arr->count + 1) > arr->capacity)) {
		if (arr->capacity == 0) {
			arr->capacity = 8;
			arr->count = 0;
		} else {
			arr->capacity *= 2;
		}

		size_t newSize = sizeof(fjsonElement **) * arr->count;

		const fjsonElement **items = (const fjsonElement **)FJSON_REALLOC(arr->items, newSize);
		arr->items = items;
	}

	size_t index = arr->count;

	arr->items[index] = child;

	arr->count++;

	return index;
}

fjson_api bool fjsonArrayRemoveChild(fjsonContext *ctx, fjsonArray *arr, const fjsonElement *child) {
	if (ctx == fjson_null || arr == fjson_null || child == fjson_null) {
		return false;
	}
}

fjson_api bool fjsonArrayClear(fjsonContext *ctx, fjsonArray *arr) {
	if (ctx == fjson_null || arr == fjson_null) {
		return false;
	}
}


fjson_api fjsonElement fjsonElementString(fjsonContext *ctx, const char *stringValue) {
	if (ctx == fjson_null) {
		return fjson__ZeroInit;
	}

	fjson__InternalContext *internalCtx = (fjson__InternalContext *)ctx;

	size_t textLength = fplGetStringLength(stringValue);

	const char *clonedText = fjson__CreateString(&internalCtx->stringPool, stringValue, textLength);
	if (clonedText == fjson_null) {
		return fjson__ZeroInit;
	}

	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_String;
	result.stringValue.text = clonedText;
	result.stringValue.length = textLength;
	return result;
}

fjson_api fjsonElement fjsonElementFloat(fjsonContext *ctx, const double floatValue) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Float;
	result.floatValue.value = floatValue;
	return result;
}

fjson_api fjsonElement fjsonElementInteger(fjsonContext *ctx, const int64_t intValue) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Integer;
	result.integerValue.value = intValue;
	return result;
}

fjson_api fjsonElement fjsonElementBool(fjsonContext *ctx, const bool boolValue) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Bool;
	result.boolValue.value = boolValue ? 1 : 0;
	return result;
}

fjson_api fjsonElement fjsonElementObject(fjsonContext *ctx, const fjsonObject *obj) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Object;
	result.objectValue = obj;
	return result;
}

fjson_api fjsonElement fjsonElementArray(fjsonContext *ctx, const fjsonArray *arr) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Array;
	result.arrayValue = arr;
	return result;
}

fjson_api fjsonElement fjsonElementNull(fjsonContext *ctx) {
	fjsonElement result = fjson__ZeroInit;
	result.type = fjsonType_Null;
	return result;
}

FJSON_CPP_END

#endif // FXML_IMPLEMENTATION