/***
final_memory.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file library for using heap memory like a stack.

The only dependencies are a C99 complaint compiler.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you want.
- Define FMEM_IMPLEMENTATION before including this header file in your main translation unit.

-------------------------------------------------------------------------------
	Usage growable memory (Default case)
-------------------------------------------------------------------------------

#define FMEM_IMPLEMENTATION
#include <final_mem.h>

fmemMemoryBlock myMem;
if (fmemInit(&myMem, fmemFlags_Growable, FMEM_MEGABYTES(16))) {
	// Uses the first memory block
	uint8_t *smallData = fmemPushSize(&myMem, FMEM_MEGABYTES(3), fmemFlags_None);

	// Adds another memory block
	uint8_t *bigData = fmemPushSize(&myMem, FMEM_MEGABYTES(64), fmemFlags_None);
	...

	// Uses the first memory block
	uint8_t *anotherBlock = fmemPushSize(&myMem, FMEM_MEGABYTES(5), fmemFlags_None);

	// Does not fit in the first block, therefore uses the second block
	uint8_t *anotherBiggerBlock = fmemPushSize(&myMem, FMEM_MEGABYTES(9), fmemFlags_None);

	// Releases all memory blocks
	fmemRelease(&myMem);
}

// ........................

fmemMemoryBlock myMem2;
if (fmemInit(&myMem2, fmemFlags_Growable, 0)) {
	// Allocates the first block, because it was not allocated beforehand
	uint8_t *data = fmemPushSize(&myMem2, FMEM_MEGABYTES(3), fmemFlags_None);

	// ...

	// Releases all memory blocks
	fmemRelease(&myMem2);
}

// ........................

// For growable memory, init is not required when it is initialized to zero
fmemMemoryBlock myMem3 = {0};

// Allocates the first block, because it was not allocated initially
uint8_t *data = fmemPushSize(&myMem3, FMEM_MEGABYTES(3), fmemFlags_None);

// ...

// Releases all memory blocks
fmemRelease(&myMem3);

-------------------------------------------------------------------------------
	Usage fixed/static memory
-------------------------------------------------------------------------------

#define FMEM_IMPLEMENTATION
#include <final_mem.h>

fmemMemoryBlock myMem;
if (fmemInit(&myMem, fmemFlags_Fixed, FMEM_MEGABYTES(16))) {
	uint32_t *data = (uint32_t *)fmemPushSize(&myMem, sizeof(uint32_t) * 10, fmemFlags_None);
	data[0] = 1;
	data[1] = 2;

	// Returns null, size does not fit in stack block
	uint8_t *bigData = fmemPushSize(&myMem, FMEM_MEGABYTES(32), fmemFlags_None);

	...

	fmemRelease(&myMem);
}

-------------------------------------------------------------------------------
	Usage temporary memory
-------------------------------------------------------------------------------

#define FMEM_IMPLEMENTATION
#include <final_mem.h>

fmemMemoryBlock myMem;
if (fmemInit(&myMem, fmemFlags_Growable, FMEM_MEGABYTES(16))) {
	uint8_t *data = fmemPushSize(&myMem, FMEM_MEGABYTES(4), fmemFlags_None);
	data[0] = 1;
	data[1] = 2;

	// Use remaining size of the source memory block
	// Source memory block is locked now and cannot be used until the temporary memory is released
	fmemMemoryBlock tempMem;
	if (fmemBeginTemporary(&myMem, &tempMem)) {
		fmemEndTemporary(&tempMem);
	}

	// Source memory is restored and unlocked
	uint8_t *moreData = fmemPushSize(&myMem, FMEM_MEGABYTES(2), fmemFlags_None);
	moreData[0] = 128;
	moreData[1] = 223;

	fmemRelease(&myMem);
}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Memory is released under the following license:

MIT License

Copyright (c) 2017-2019 Torsten Spaete

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
	\file final_memory.h
	\version v0.2.1 alpha
	\author Torsten Spaete
	\brief Final Memory (FMEM) - A open source C99 single file header memory library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

    ## v0.2.1 alpha:
    - Fixed: Two inline functions was not found in GCC

    ## v0.2 alpha:
	- Added: New function fmemGetHeader
	- Added: Safety check for fmemBeginTemporary when passing a temporary block as source
	- Fixed: Memory is no longer wasted
	- Fixed: Fixed crash when fmemGetTotalSize was passed a fixed-size block
	- Fixed: Fixed crash when fmemGetRemainingSize was passed a fixed-size block
	- Changed: fmemType_Fixed does not allocate in sized-block anymore, instead it uses the desired size + meta size

	## v0.1.0 alpha:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- Removal of single memory blocks

*/

#ifndef FMEM_H
#define FMEM_H

// Detect compiler
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
#	define FMEM_IS_C99
#elif defined(__cplusplus)
#	define FMEM_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

// Api export
#if defined(FMEM_PRIVATE)
#	define fmem_api static
#else
#	define fmem_api extern
#endif

#if defined(FMEM_IS_C99)
	//! Initialize a struct to zero (C99)
#	define FMEM_ZERO_INIT {0}
#else
	//! Initialize a struct to zero (C++)
#	define FMEM_ZERO_INIT {}
#endif

// Includes
#include <stdint.h> // int32_t, etc.
#include <stdbool.h> // bool
#include <stddef.h> // NULL

// Functions override
#ifndef FMEM_MEMSET
#	include <string.h>
#	define FMEM_MEMSET memset
#endif
#ifndef FMEM_MALLOC
#	include <malloc.h>
#	define FMEM_MALLOC malloc
#	define FMEM_FREE free
#endif
#ifndef FMEM_ASSERT
#	include <assert.h>
#	define FMEM_ASSERT assert
#endif

//! Null pointer
#define fmem_null NULL

//! Returns the number of bytes for the given kilobytes
#define FMEM_KILOBYTES(value) (((value) * 1024ull))
//! Returns the number of bytes for the given megabytes
#define FMEM_MEGABYTES(value) ((FMEM_KILOBYTES(value) * 1024ull))
//! Returns the number of bytes for the given gigabytes
#define FMEM_GIGABYTES(value) ((FMEM_MEGABYTES(value) * 1024ull))
//! Returns the number of bytes for the given terabytes
#define FMEM_TERABYTES(value) ((FMEM_GIGABYTES(value) * 1024ull))

typedef enum fmemPushFlags {
	//! No push flags
	fmemPushFlags_None = 0,
	//! Clear region to zero
	fmemPushFlags_Clear = 1 << 0,
} fmemPushFlags;

typedef enum fmemType {
	//! Growable block
	fmemType_Growable = 0,
	//! Fixed size block
	fmemType_Fixed,
	//! Temporary block
	fmemType_Temporary,
} fmemType;

typedef enum fmemSizeFlags {
	//! No size flags
	fmemSizeFlags_None = 0,
	//! Returns the size for a single block only
	fmemSizeFlags_Single = 1 << 0,
	//! Include size with meta-data
	fmemSizeFlags_WithMeta = 1 << 1,
} fmemSizeFlags;

typedef struct fmemBlockHeader {
	//! Previous block
	struct fmemMemoryBlock *prev;
	//! Next block
	struct fmemMemoryBlock *next;
} fmemBlockHeader;

typedef struct fmemMemoryBlock {
	//! Source memory pointer if present
	void *source;
	//! Base memory pointer
	void *base;
	//! Pointer to a temporary memory block
	struct fmemMemoryBlock *temporary;
	//! Total size in bytes
	size_t size;
	//! Used size in bytes
	size_t used;
	//! Type
	fmemType type;
} fmemMemoryBlock;

//! Initializes the given block or allocates memory when size is greater than zero
fmem_api bool fmemInit(fmemMemoryBlock *block, const fmemType type, const size_t size);
//! Initializes the given block to a fixed size block from existing source memory
fmem_api bool fmemInitFromSource(fmemMemoryBlock *block, void *sourceMemory, const size_t sourceSize);
//! Release this and all appended memory blocks
fmem_api void fmemFree(fmemMemoryBlock *block);
//! Gets memory from the block by the given size
fmem_api uint8_t *fmemPush(fmemMemoryBlock *block, const size_t size, const fmemPushFlags flags);
//! Gets memory from the block by the given size and ensure address alignment
fmem_api uint8_t *fmemPushAligned(fmemMemoryBlock *block, const size_t size, const size_t alignment, const fmemPushFlags flags);
//! Gets memory for a new block with the given size
fmem_api bool fmemPushBlock(fmemMemoryBlock *src, fmemMemoryBlock *dst, const size_t size, const fmemPushFlags flags);
//! Returns the remaining size of all blocks starting by the given block
fmem_api size_t fmemGetRemainingSize(fmemMemoryBlock *block);
//! Returns the total size of all blocks starting by the given block
fmem_api size_t fmemGetTotalSize(fmemMemoryBlock *block);
//! Resets the given block usage to zero without freeing any memory
fmem_api void fmemReset(fmemMemoryBlock *block);
//! Initializes a temporary block with the remaining size of the source block
fmem_api bool fmemBeginTemporary(fmemMemoryBlock *source, fmemMemoryBlock *temporary);
//! Gives the memory back to source block from the temporary block
fmem_api void fmemEndTemporary(fmemMemoryBlock *temporary);
//! Returns the block header pointer for the given block
fmem_api fmemBlockHeader *fmemGetHeader(fmemMemoryBlock *block);

#endif // FMEM_H

#if defined(FMEM_IMPLEMENTATION) && !defined(FMEM_IMPLEMENTED)
#define FMEM_IMPLEMENTED

//! Default spacing after the header
#define FMEM__HEADER_SPACING sizeof(uintptr_t)
//! Default block size = Page size
#define FMEM__MIN_BLOCKSIZE 4096
//! Size of the meta data for the block (Header+Spacing+Block+Spacing)
#define FMEM__BLOCK_META_SIZE sizeof(fmemBlockHeader) + FMEM__HEADER_SPACING + sizeof(fmemMemoryBlock) + FMEM__HEADER_SPACING
//! Offset to block from the header
#define FMEM__OFFSET_TO_BLOCK sizeof(fmemBlockHeader) + FMEM__HEADER_SPACING
//! Returns the header from the given block
#define FMEM__GETHEADER(block) (fmemBlockHeader *)((uint8_t *)(block)->base - (FMEM__BLOCK_META_SIZE))
//! Returns the header from the given block
#define FMEM__GETBLOCK(header) (fmemMemoryBlock *)((uint8_t *)(header) + FMEM__OFFSET_TO_BLOCK)

static size_t fmem__GetSpaceAvailableFor(const fmemMemoryBlock *block, const size_t size) {
	size_t result = ((block->size > 0) && (block->used <= block->size)) ? ((block->size - block->used) - size) : 0;
	return(result);
}

static size_t fmem__ComputeBlockSize(size_t size) {
	FMEM_ASSERT(size >= FMEM__BLOCK_META_SIZE);
	size_t count = (size / FMEM__MIN_BLOCKSIZE) + 1;
	size_t result = count * FMEM__MIN_BLOCKSIZE;
	return(result);
}

static fmemBlockHeader *fmem__AllocateBlock(const size_t blockSize) {
	FMEM_ASSERT(blockSize >= FMEM__BLOCK_META_SIZE);
	void *base = FMEM_MALLOC(blockSize);
	if(base == fmem_null) {
		return fmem_null;
	}
	fmemBlockHeader *header = (fmemBlockHeader *)base;
	FMEM_MEMSET(header, 0, sizeof(*header));
	fmemMemoryBlock *nextBlock = FMEM__GETBLOCK(header);
	FMEM_MEMSET(nextBlock, 0, sizeof(*nextBlock));
	return(header);
}

static void fmem__FreeBlock(fmemBlockHeader *header) {
	FMEM_ASSERT(header != fmem_null);
	FMEM_FREE(header);
}

fmem_api fmemBlockHeader *fmemGetHeader(fmemMemoryBlock *block) {
	if(block == fmem_null) {
		return fmem_null;
	}
	if(block->base == fmem_null) {
		return fmem_null;
	}
	fmemBlockHeader *header = (fmemBlockHeader *)block->base;
	return(header);
}

fmem_api size_t fmemGetRemainingSize(fmemMemoryBlock *block) {
	if(block == fmem_null) {
		return 0;
	}
	size_t result = 0;
	fmemMemoryBlock *testBlock = block;
	while(testBlock != fmem_null) {
		if(testBlock->base == fmem_null || testBlock->size == 0) {
			break;
		}
		result += fmem__GetSpaceAvailableFor(testBlock, 0);
		if(testBlock->type != fmemType_Growable) {
			break;
		}
		fmemBlockHeader *header = FMEM__GETHEADER(testBlock);
		testBlock = header->next;
	}
	return(result);
}

fmem_api size_t fmemGetTotalSize(fmemMemoryBlock *block) {
	if(block == fmem_null) {
		return 0;
	}
	size_t result = 0;
	fmemMemoryBlock *testBlock = block;
	while(testBlock != fmem_null) {
		if(testBlock->base == fmem_null || testBlock->size == 0) {
			break;
		} 
		size_t sizeForBlock = testBlock->size;
		result += sizeForBlock;
		if(testBlock->type != fmemType_Growable) {
			break;
		}
		fmemBlockHeader *header = FMEM__GETHEADER(testBlock);
		testBlock = header->next;
	}
	return(result);
}

fmem_api bool fmemInit(fmemMemoryBlock *block, const fmemType type, const size_t size) {
	if(block == fmem_null) {
		return(false);
	}
	if(type == fmemType_Fixed && size == 0) {
		return(false);
	}
	if(type == fmemType_Temporary) {
		return(false);
	}
	FMEM_MEMSET(block, 0, sizeof(*block));
	block->type = type;
	if(size > 0) {
		size_t blockSize;
		size_t metaSize = FMEM__BLOCK_META_SIZE;
		if(type == fmemType_Fixed) {
			blockSize = size + metaSize;
		} else {
			blockSize = fmem__ComputeBlockSize(size + metaSize);
		}
		fmemBlockHeader *header = fmem__AllocateBlock(blockSize);
		if(header == fmem_null) {
			return(false);
		}
		block->base = (uint8_t *)header + metaSize;
		block->size = blockSize - metaSize;
	}
	return(true);
}

fmem_api bool fmemInitFromSource(fmemMemoryBlock *block, void *sourceMemory, const size_t sourceSize) {
	if((block == fmem_null) || (sourceSize == 0)) {
		return(false);
	}
	if(sourceMemory == fmem_null || sourceSize == 0) {
		return(false);
	}
	FMEM_MEMSET(block, 0, sizeof(*block));
	block->type = fmemType_Fixed;
	block->base = sourceMemory;
	block->size = sourceSize;
	block->source = sourceMemory;
	return(true);
}

fmem_api void fmemFree(fmemMemoryBlock *block) {
	if(block != fmem_null && block->temporary == fmem_null) {
		fmemMemoryBlock *freeBlock = block;
		while(freeBlock != fmem_null) {
			if(freeBlock->base == fmem_null || freeBlock->size == 0 || freeBlock->source != fmem_null) {
				break;
			}
			fmemBlockHeader *header = FMEM__GETHEADER(freeBlock);
			fmemMemoryBlock *next = header->next;
			fmem__FreeBlock(header);
			freeBlock = next;
		}
		FMEM_MEMSET(block, 0, sizeof(*block));
	}
}

fmem_api uint8_t *fmemPush(fmemMemoryBlock *block, const size_t size, const fmemPushFlags flags) {
	if(block == fmem_null || size == 0) {
		return fmem_null;
	}
	if(block->temporary != fmem_null) {
		return fmem_null;
	}

	// Find best fitting block (Most space available after append)
	fmemMemoryBlock *bestBlock = fmem_null;
	fmemMemoryBlock *searchBlock = block;
	while(searchBlock != fmem_null) {
		if(searchBlock->base == fmem_null || searchBlock->size == 0) {
			break;
		}
		if((searchBlock->used + size) <= searchBlock->size) {
			if(bestBlock == fmem_null || (fmem__GetSpaceAvailableFor(searchBlock, size) > fmem__GetSpaceAvailableFor(bestBlock, size))) {
				bestBlock = searchBlock;
			}
		}
		fmemBlockHeader *header = FMEM__GETHEADER(searchBlock);
		if(searchBlock->type != fmemType_Growable) {
			break;
		}
		searchBlock = header->next;
	}

	// Stupid compiler, i dont care about crosses initialization for labels
	fmemMemoryBlock *newBlock;
	fmemMemoryBlock *tailBlock;
	fmemBlockHeader *newHeader;
	fmemBlockHeader *tailHeader;
	size_t blockSize;

	// @NOTE(final): Do not initialize, because i want all code paths below to set a result
	uint8_t *result;

	if(bestBlock != fmem_null) {
		result = (uint8_t *)bestBlock->base + bestBlock->used;
		bestBlock->used += size;
		goto done;
	} else {
		if(block->type != fmemType_Growable) {
			result = fmem_null;
			goto done;
		}
	}

	// Find tail block to append on
	tailBlock = fmem_null;
	if(block->base != fmem_null) {
		tailBlock = block;
		while(tailBlock != fmem_null) {
			fmemBlockHeader *thisHeader = FMEM__GETHEADER(tailBlock);
			if(thisHeader->next == fmem_null) {
				break;
			}
			tailBlock = thisHeader->next;
		}
	}

	// Allocate new block
	blockSize = fmem__ComputeBlockSize(size + FMEM__BLOCK_META_SIZE);
	newHeader = fmem__AllocateBlock(blockSize);
	if(newHeader == fmem_null) {
		result = fmem_null;
		goto done;
	}

	if(tailBlock == fmem_null) {
		// No tail found -> Setup block argument
		block->size = blockSize - FMEM__BLOCK_META_SIZE;
		block->base = (uint8_t *)newHeader + FMEM__BLOCK_META_SIZE;
		block->used = size;
		block->source = fmem_null;
		result = (uint8_t *)block->base;
		goto done;
	}

	// Setup next block
	newBlock = FMEM__GETBLOCK(newHeader);
	newBlock->base = (uint8_t *)newHeader + FMEM__BLOCK_META_SIZE;
	newBlock->size = blockSize - FMEM__BLOCK_META_SIZE;
	newBlock->type = tailBlock->type;
	newBlock->source = fmem_null;
	newBlock->used = size;

	// Append next block to tail
	newHeader->prev = tailBlock;
	tailHeader = FMEM__GETHEADER(tailBlock);
	tailHeader->next = newBlock;

	result = (uint8_t *)newBlock->base;
done:
	if(result != fmem_null) {
		if(flags & fmemPushFlags_Clear) {
			FMEM_MEMSET(result, 0, size);
		}
	}
	return(result);
}

fmem_api uint8_t *fmemPushAligned(fmemMemoryBlock *block, const size_t size, const size_t alignment, const fmemPushFlags flags) {
	uint8_t *result = fmem_null;
	return(result);
}

fmem_api bool fmemPushBlock(fmemMemoryBlock *src, fmemMemoryBlock *dst, const size_t size, const fmemPushFlags flags) {
	if(src == fmem_null || dst == fmem_null || size == 0) {
		return(false);
	}
	uint8_t *base = fmemPush(src, size, flags);
	if(base == fmem_null) {
		return(false);
	}
	dst->base = base;
	dst->size = size;
	dst->used = 0;
	dst->source = src;
	dst->type = fmemType_Fixed;
	return(true);
}

fmem_api void fmemReset(fmemMemoryBlock *block) {
	if(block != fmem_null && block->temporary == fmem_null) {
		block->used = 0;
	}
}

fmem_api bool fmemBeginTemporary(fmemMemoryBlock *source, fmemMemoryBlock *temporary) {
	if(source == fmem_null || temporary == fmem_null) {
		return(false);
	}
	if(source->base == fmem_null || source->size == 0) {
		return(false);
	}
	size_t remainingSize = fmemGetRemainingSize(source);
	if(remainingSize == 0) {
		return(false);
	}
	FMEM_MEMSET(temporary, 0, sizeof(*temporary));
	temporary->base = (uint8_t *)source->base + source->used;
	temporary->size = remainingSize;
	temporary->used = 0;
	temporary->source = source;
	temporary->type = fmemType_Temporary;
	temporary->temporary = fmem_null;
	source->used += remainingSize;
	source->temporary = temporary;
	FMEM_ASSERT(source->used == source->size);
	return(true);
}

fmem_api void fmemEndTemporary(fmemMemoryBlock *temporary) {
	if(temporary == fmem_null) {
		return;
	}
	if(temporary->type != fmemType_Temporary || temporary->source == fmem_null || temporary->size == 0) {
		return;
	}
	fmemMemoryBlock *sourceBlock = (fmemMemoryBlock *)temporary->source;
	FMEM_ASSERT(sourceBlock->temporary == temporary);
	FMEM_ASSERT(sourceBlock->used == sourceBlock->size);
	FMEM_ASSERT(temporary->size <= sourceBlock->size);
	sourceBlock->temporary = fmem_null;
	sourceBlock->used -= temporary->size;
	FMEM_MEMSET(temporary, 0, sizeof(*temporary));
}

#endif // FMEM_IMPLEMENTATION