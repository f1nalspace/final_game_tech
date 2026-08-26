/***
final_tiletrace.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

An open source single header file contour tile tracing C17 library.

This creates chainshapes out of any solid tilemap based on a contour tracing algorithm.
It will try to create as few chain shapes as possible - useful for generating
collision shapes (e.g. Box2D b2ChainShape) out of a solid block tilemap.

This is a pure C17 rewrite of the original C++ "final_tiletrace.hpp" library.
It has no dependencies other than the C standard library and uses a growing
arena allocator with a pluggable memory allocator (defaults to malloc/free).

- Supports full block tiles and the four diagonal slope tiles (see fttTileType).

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your C/C++ project and include it.
- Define FTT_IMPLEMENTATION before including this header in ONE translation unit.
- Initialize a tracer with fttInitTileTracer().
- Either step through with fttNextTileTraceStep() (to visualize each step) or run
  to completion with fttRunTileTracer().
- Inspect the public tracer state (vertices, edges, chain segments) for your use.
- Release all memory with fttFreeTileTracer().

-------------------------------------------------------------------------------
	Usage
-------------------------------------------------------------------------------

#define FTT_IMPLEMENTATION
#include <final_tiletrace.h>

uint8_t map[width * height] = { ... };           // 0 = empty, 1 = solid, 2..5 = slopes (fttTileType)
fttVec2u tileCount = { width, height };

fttTileTracer tracer;
fttInitTileTracer(&tracer, tileCount, map, ftt_null); // ftt_null = default malloc/free
fttRunTileTracer(&tracer);

for (size_t i = 0; i < fttGetChainSegmentCount(&tracer); ++i) {
	const fttChainSegment *seg = fttGetChainSegment(&tracer, i);
	for (size_t v = 0; v < fttGetChainSegmentVertexCount(seg); ++v) {
		fttVec2i p = fttGetChainSegmentVertex(seg, v);
		// ... use p.x, p.y
	}
}

fttFreeTileTracer(&tracer);

-------------------------------------------------------------------------------
	Custom allocator
-------------------------------------------------------------------------------

fttAllocator allocator;
allocator.allocate = MyAlloc;   // void *(*)(void *userData, size_t size)
allocator.release  = MyFree;    // void  (*)(void *userData, void *ptr)
allocator.userData = myContext;
fttInitTileTracer(&tracer, tileCount, map, &allocator);

-------------------------------------------------------------------------------
	Preprocessor overrides
-------------------------------------------------------------------------------

FTT_IMPLEMENTATION       Define in ONE translation unit to emit the implementation.
FTT_API_AS_PRIVATE       Set to 1 to make the public api static (single file only).
FTT_ARENA_MIN_BLOCK_SIZE Minimum arena block size in bytes (default 65536 = 64 KB).
FTT_ASSERT(expr)         Override the assertion macro (defaults to assert from <assert.h>).
FTT_MALLOC(size)         Override the default allocate (defaults to malloc; skips <stdlib.h> when set with FTT_FREE).
FTT_FREE(ptr)            Override the default release (defaults to free; skips <stdlib.h> when set with FTT_MALLOC).
FTT_MEMCPY(dst,src,size) Override memory copy (defaults to memcpy; skips <string.h> when set with FTT_MEMMOVE).
FTT_MEMMOVE(dst,src,size)Override memory move (defaults to memmove; skips <string.h> when set with FTT_MEMCPY).

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final TileTrace is released under the following license:

MIT License

Copyright (c) 2017-2026 Torsten Spaete

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
	@file final_tiletrace.h
	@version v2.0.1
	@author Torsten Spaete
	@brief Final TileTrace (FTT) - A pure C17 single file header contour tile tracing library.
*/

// ****************************************************************************
//
// > Changelog
//
// ****************************************************************************

/*!
	@page page_changelog Changelog
	@tableofcontents

	# v2.0.1:
	- Fixed: Arena allocation failure (OOM) no longer dereferences a null block in release; the arena sets outOfMemory, pushes fall back to a scratch slot, and the tracer stops cleanly

	# v2.0.0:
	- New: Full rewrite from C++ to pure C17
	- New: Growing arena allocator with a pluggable memory allocator (default malloc/free)
	- New: Custom standalone math types (no external math dependency)
	- New: Integer progress tracking (fttGetProgressPercentage returns a 0..100 percentage)
	- New: Slope tile support - map values 2..5 map to the four diagonal slopes in fttTileType
	- New: fttGetTileShapeCorners returns the clockwise polygon corners for any tile type (4 solid, 3 slope)
	- Changed: The C++ class api was replaced by a plain C api (ftt prefix)
	- Changed: Edge/segment references use indices instead of raw pointers (realloc-safe)

	# v1.0.2:
	- Documented algorithm in broad details (C++ version)

	# v1.0.1:
	- Added additional C++ api

	# v1.0:
	- Initial version
*/

#ifndef FTT_INCLUDE_H
#define FTT_INCLUDE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(FTT_API_AS_PRIVATE)
#	define FTT_API_AS_PRIVATE 0
#endif

#if FTT_API_AS_PRIVATE
#	define ftt_api static
#else
#	define ftt_api extern
#endif // FTT_API_AS_PRIVATE

//! Null pointer
#define ftt_null NULL

//! Inline helper
#define ftt_inline static inline

#if !defined(FTT_ARENA_MIN_BLOCK_SIZE)
//! Default minimum arena block size (64 KB)
#	define FTT_ARENA_MIN_BLOCK_SIZE 65536
#endif

//! Assertion macro - define FTT_ASSERT before including to override the default (assert)
#if !defined(FTT_ASSERT)
#	include <assert.h>
#	define FTT_ASSERT(expression) assert(expression)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************
//
// > Math types
//
// ****************************************************************************

//! 2D signed 32-bit integer vector
typedef union fttVec2i {
	struct {
		int32_t x;
		int32_t y;
	};
	struct {
		int32_t w;
		int32_t h;
	};
	int32_t e[2];
} fttVec2i;

//! 2D unsigned 32-bit integer vector
typedef union fttVec2u {
	struct {
		uint32_t x;
		uint32_t y;
	};
	struct {
		uint32_t w;
		uint32_t h;
	};
	uint32_t e[2];
} fttVec2u;

// ****************************************************************************
//
// > Memory (allocator + growing arena + dynamic array)
//
// ****************************************************************************

//! Pluggable memory allocator. Hands out large blocks; the arena sub-allocates from them.
typedef struct fttAllocator {
	//! Allocates a block of at least the given size
	void *(*allocate)(void *userData, size_t size);
	//! Releases a block previously returned by allocate
	void (*release)(void *userData, void *ptr);
	//! User supplied context passed to allocate/release
	void *userData;
} fttAllocator;

//! A single block of memory inside the arena (linked list node)
typedef struct fttMemoryBlock {
	uint8_t *base;                //!< Start of the usable data region
	size_t size;                  //!< Size of the usable data region in bytes
	size_t used;                  //!< Number of bytes used so far
	struct fttMemoryBlock *next;  //!< Next block in the chain
} fttMemoryBlock;

//! A growing bump allocator that chains memory blocks as it needs more space
typedef struct fttArena {
	fttMemoryBlock *first;        //!< First block in the chain
	fttMemoryBlock *current;      //!< Block bytes are currently bumped from
	fttAllocator allocator;       //!< Backing allocator used to request new blocks
	size_t minBlockSize;          //!< Minimum size for newly requested blocks
	bool outOfMemory;             //!< Set when the backing allocator failed; the tracer stops instead of crashing
	//! Fallback slot returned for a single element push when out of memory, so the immediate write never touches null (element is discarded). Sized to hold the largest tracer element.
	uint8_t oomSink[128];
} fttArena;

//! A growable, arena backed, type-erased dynamic array
typedef struct fttArray {
	void *items;                  //!< Pointer to the contiguous element storage
	size_t count;                 //!< Number of valid elements
	size_t capacity;              //!< Number of elements that fit without growing
	size_t itemSize;              //!< Size of a single element in bytes
	fttArena *arena;              //!< Arena the storage is allocated from
} fttArray;

//! Returns a typed element value from a fttArray
#define FTT_ARRAY_AT(a, T, i) (((T *)(a).items)[i])
//! Returns a typed element pointer from a fttArray
#define FTT_ARRAY_PTR(a, T, i) (&((T *)(a).items)[i])

// ****************************************************************************
//
// > Tracer types
//
// ****************************************************************************

typedef enum fttTileType {
	// Empty Space
	FTT_TILE_EMPTY = 0,

	// Solid Tile
	// ┌─────┐
	// │     │
	// │     │
	// │     │
	// │     │
	// │     │
	// └─────┘
	FTT_TILE_SOLID = 1,

	// Bottom-Right Slope (surface facing top-left)
	//       x
	//      /│
	//     / │
	//    /  │
	//   /   │
	//  /    │
	// x─────┘
	FTT_TILE_SLOPE_BOTTOM_RIGHT = 2,

	// Bottom-Left Slope (surface facing top-right)
	// x
	// │\
	// │ \
	// │  \
	// │   \
	// │    \
	// └─────x
	FTT_TILE_SLOPE_BOTTOM_LEFT = 3,

	// Top-Left Slope (surface facing bottom-right)
	// ┌─────x
	// │    ╱
	// │   ╱
	// │  ╱
	// │ ╱
	// │╱
	// x
	FTT_TILE_SLOPE_TOP_LEFT = 4,

	// Top-Right Slope  (surface facing bottom-left)
	// x─────┐
	//  \    │
	//   \   │
	//    \  │
	//     \ │
	// 	    \│
	//       x
	FTT_TILE_SLOPE_TOP_RIGHT = 5,
} fttTileType;

//! Search direction (indexes the internal direction table)
typedef enum fttDirection {
	FTT_DIRECTION_UP = 0,
	FTT_DIRECTION_RIGHT,
	FTT_DIRECTION_DOWN,
	FTT_DIRECTION_LEFT,
	FTT_DIRECTION_COUNT,
} fttDirection;

//! Trace step of the state machine
typedef enum fttStep {
	FTT_STEP_NONE = 0,
	FTT_STEP_FIND_START,
	FTT_STEP_GET_NEXT_OPEN_TILE,
	FTT_STEP_FIND_NEXT_TILE,
	FTT_STEP_ROTATE_FORWARD,
	FTT_STEP_TRAVERSE_FIND_STARTING_EDGE,
	FTT_STEP_TRAVERSE_NEXT_EDGE,
	FTT_STEP_DONE,
} fttStep;

//! A single tile in the map. isSolid > 0 means solid, 0 means empty, -1 means visited/removed.
typedef struct fttTile {
	int32_t x, y;             //!< Tile grid coordinates
	uint32_t traceDirection;  //!< Current scan direction index (0-3)
	int32_t isSolid;          //!< >0 solid, 0 empty, -1 visited/removed
	fttTileType tileType;     //!< Original tile type (solid or one of the slopes) - persists after removal
} fttTile;

//! A directed boundary edge between two vertices in mainVertices (wound clockwise around a solid tile).
typedef struct fttEdge {
	int32_t index;        //!< Local edge index within the tile (0-3: left, top, right, bottom)
	int32_t vertIndex0;   //!< Start vertex index into fttTileTracer::mainVertices
	int32_t vertIndex1;   //!< End vertex index into fttTileTracer::mainVertices
	fttVec2i tilePosition;//!< Grid position of the tile this edge originated from
	bool isInvalid;       //!< True once this edge has been consumed by phase 2 traversal
} fttEdge;

//! The world-space corner positions for a single tile, wound clockwise (screen space, y-down).
//! A solid tile has four corners (bottom-left, top-left, top-right, bottom-right); a slope has three.
typedef struct fttTileVertices {
	fttVec2i verts[4];  //!< Corner positions (only entries [0..count-1] are valid)
	uint32_t count;     //!< Number of valid corners (4 for solid, 3 for slopes)
} fttTileVertices;

//! Indices into fttTileTracer::mainVertices for each of a tile's corners.
typedef struct fttTileIndices {
	int32_t indices[4];  //!< Vertex indices (only entries [0..count-1] are valid)
	uint32_t count;      //!< Number of valid indices (4 for solid, 3 for slopes)
} fttTileIndices;

//! Up to four directed edges produced for a single tile, with the count of valid entries.
typedef struct fttTileEdges {
	fttEdge edges[4];  //!< Edge array (only entries [0..count-1] are valid)
	uint32_t count;    //!< Number of valid edges in the array
} fttTileEdges;

//! A single output contour - an ordered list of world-space vertices (fttVec2i).
//! Closed contours have the first vertex duplicated at the end.
typedef struct fttChainSegment {
	fttArray vertices;  //!< Dynamic array of fttVec2i
} fttChainSegment;

//! Full mutable state of the tile tracer. All fields are public for inspection/visualization.
typedef struct fttTileTracer {
	fttAllocator allocator;       //!< Resolved allocator (default used when caller passed none)
	fttArena arena;               //!< Owns all dynamic memory below

	fttVec2u tileCount;           //!< Width and height of the tile grid
	fttArray tiles;               //!< Flat row-major fttTile array (modified in place)
	fttStep curStep;              //!< Current state-machine step

	fttTile *startTile;           //!< Start tile of the current expansion region
	fttTile *curTile;             //!< Tile currently being expanded from
	fttTile *nextTile;            //!< Candidate neighbour tile (transient)

	int32_t startEdgeIndex;       //!< Index into mainEdges of the first edge of the current segment (-1 = none)
	int32_t lastEdgeIndex;        //!< Index into mainEdges of the most recently consumed edge (-1 = none)
	int32_t curChainSegmentIndex; //!< Index into chainSegments of the segment being built (-1 = none)

	fttArray openList;            //!< LIFO stack of fttTile* waiting to be expanded (phase 1)
	fttArray mainVertices;        //!< Deduplicated fttVec2i pool shared across all tiles
	fttArray mainEdges;           //!< fttEdge boundary edges (phase 1 output / phase 2 input)
	fttArray chainSegments;       //!< fttChainSegment final output (one per contiguous solid-region boundary)

	// Progress tracking - integer counters used to compute a 0..100 percentage
	uint32_t totalSolidTileCount; //!< Number of solid tiles at init (phase 1 denominator)
	uint32_t processedTileCount;  //!< Number of solid tiles expanded so far (phase 1 numerator)
	uint32_t totalEdgeCount;      //!< Number of boundary edges when edge traversal begins (phase 2 denominator)
	uint32_t consumedEdgeCount;   //!< Number of boundary edges consumed into chain segments so far (phase 2 numerator)
} fttTileTracer;

// ****************************************************************************
//
// > Public api
//
// ****************************************************************************

//! Returns the default malloc/free based allocator
ftt_api fttAllocator fttDefaultAllocator(void);

//! Initializes a tile tracer for the given tile map. Pass ftt_null as allocator to use the default.
ftt_api void fttInitTileTracer(fttTileTracer *tracer, fttVec2u tileCount, const uint8_t *mapTiles, const fttAllocator *allocator);
//! Executes the next step for the given tracer. Returns false when the tracer is done.
ftt_api bool fttNextTileTraceStep(fttTileTracer *tracer);
//! Runs the full tracer until it is done.
ftt_api void fttRunTileTracer(fttTileTracer *tracer);
//! Releases all memory owned by the tracer.
ftt_api void fttFreeTileTracer(fttTileTracer *tracer);
//! Gets the progress percentage of the current state of the tracer.
ftt_inline float fttGetProgressPercentage(const fttTileTracer *tracer);

//! Fills outCorners with the clockwise (screen space, y-down) grid-space polygon corners for a tile of the given type at (x, y).
//! Returns the number of corners written: 4 for a solid tile, 3 for a slope, 0 for an empty tile.
//! Useful for rendering or building collision shapes that match the traced contour.
ftt_api uint32_t fttGetTileShapeCorners(fttTileType tileType, int32_t x, int32_t y, fttVec2i outCorners[4]);

// ****************************************************************************
//
// > Public inline accessors
//
// ****************************************************************************

//! Returns the number of chain segments
ftt_inline size_t fttGetChainSegmentCount(const fttTileTracer *tracer) {
	return tracer->chainSegments.count;
}
//! Returns a chain segment by the given index
ftt_inline const fttChainSegment *fttGetChainSegment(const fttTileTracer *tracer, size_t index) {
	return FTT_ARRAY_PTR(tracer->chainSegments, fttChainSegment, index);
}
//! Returns the number of vertices in a chain segment
ftt_inline size_t fttGetChainSegmentVertexCount(const fttChainSegment *segment) {
	return segment->vertices.count;
}
//! Returns a vertex of a chain segment by the given index
ftt_inline fttVec2i fttGetChainSegmentVertex(const fttChainSegment *segment, size_t index) {
	return FTT_ARRAY_AT(segment->vertices, fttVec2i, index);
}
//! Returns the number of vertices in the main vertex pool
ftt_inline size_t fttGetVertexCount(const fttTileTracer *tracer) {
	return tracer->mainVertices.count;
}
//! Returns a vertex from the main vertex pool by the given index
ftt_inline fttVec2i fttGetVertex(const fttTileTracer *tracer, size_t index) {
	return FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, index);
}
//! Returns the number of edges
ftt_inline size_t fttGetEdgeCount(const fttTileTracer *tracer) {
	return tracer->mainEdges.count;
}
//! Returns an edge by the given index
ftt_inline const fttEdge *fttGetEdge(const fttTileTracer *tracer, size_t index) {
	return FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, index);
}
//! Returns a tile by the given coordinates
ftt_inline const fttTile *fttGetTile(const fttTileTracer *tracer, uint32_t x, uint32_t y) {
	return FTT_ARRAY_PTR(tracer->tiles, fttTile, y * tracer->tileCount.w + x);
}
//! Returns the number of open tiles
ftt_inline size_t fttGetOpenTileCount(const fttTileTracer *tracer) {
	return tracer->openList.count;
}
//! Returns an open tile pointer by the given index
ftt_inline fttTile *fttGetOpenTile(const fttTileTracer *tracer, size_t index) {
	return FTT_ARRAY_AT(tracer->openList, fttTile *, index);
}
//! Returns the start tile pointer (may be null)
ftt_inline fttTile *fttGetStartTile(const fttTileTracer *tracer) {
	return tracer->startTile;
}
//! Returns the current tile pointer (may be null)
ftt_inline fttTile *fttGetCurrentTile(const fttTileTracer *tracer) {
	return tracer->curTile;
}
//! Returns the total number of solid tiles the tracer started with
ftt_inline uint32_t fttGetTotalSolidTileCount(const fttTileTracer *tracer) {
	return tracer->totalSolidTileCount;
}
//! Returns the number of solid tiles expanded so far
ftt_inline uint32_t fttGetProcessedTileCount(const fttTileTracer *tracer) {
	return tracer->processedTileCount;
}
//! Returns the overall trace progress as a percentage in the range 0.0 .. 100.0.
//! Tile expansion (phase 1) fills the first half, edge traversal (phase 2) fills the second half.
ftt_inline float fttGetProgressPercentage(const fttTileTracer *tracer) {
	const float fullPercentage = 100.0f;
	const float phase1SharePercentage = 50.0f;                                  // Tile expansion covers the first half
	const float phase2SharePercentage = fullPercentage - phase1SharePercentage; // Edge traversal covers the rest

	// An empty map has nothing to trace and is trivially complete
	if (tracer->totalSolidTileCount == 0) {
		return fullPercentage;
	}
	if (tracer->curStep == FTT_STEP_DONE) {
		return fullPercentage;
	}

	float phase1Fraction = (float)tracer->processedTileCount / (float)tracer->totalSolidTileCount;
	float phase1Percentage = phase1Fraction * phase1SharePercentage;

	float phase2Percentage = 0.0f;
	if (tracer->totalEdgeCount > 0) {
		float phase2Fraction = (float)tracer->consumedEdgeCount / (float)tracer->totalEdgeCount;
		phase2Percentage = phase2Fraction * phase2SharePercentage;
	}

	float result = phase1Percentage + phase2Percentage;
	if (result > fullPercentage) {
		result = fullPercentage;
	}
	return result;
}

#ifdef __cplusplus
}
#endif

#endif // FTT_INCLUDE_H

// ****************************************************************************
// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
// ****************************************************************************
#if defined(FTT_IMPLEMENTATION) && !defined(FTT_IMPLEMENTED)
#	define FTT_IMPLEMENTED

// Default allocator backing - only pull in <stdlib.h> when malloc/free are not overridden
#if !defined(FTT_MALLOC) || !defined(FTT_FREE)
#	include <stdlib.h>
#	if !defined(FTT_MALLOC)
#		define FTT_MALLOC(size) malloc(size)
#	endif
#	if !defined(FTT_FREE)
#		define FTT_FREE(ptr) free(ptr)
#	endif
#endif

// Memory copy/move - only pull in <string.h> when memcpy/memmove are not overridden
#if !defined(FTT_MEMCPY) || !defined(FTT_MEMMOVE)
#	include <string.h>
#	if !defined(FTT_MEMCPY)
#		define FTT_MEMCPY(dst, src, size) memcpy(dst, src, size)
#	endif
#	if !defined(FTT_MEMMOVE)
#		define FTT_MEMMOVE(dst, src, size) memmove(dst, src, size)
#	endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//! Memory alignment used by the arena
#define FTT__ALIGNMENT 16

// ----------------------------------------------------------------------------
// Default allocator (malloc/free)
// ----------------------------------------------------------------------------

static void *ftt__DefaultAllocate(void *userData, size_t size) {
	(void)userData;
	return FTT_MALLOC(size);
}
static void ftt__DefaultRelease(void *userData, void *ptr) {
	(void)userData;
	FTT_FREE(ptr);
}

ftt_api fttAllocator fttDefaultAllocator(void) {
	fttAllocator result;
	result.allocate = ftt__DefaultAllocate;
	result.release = ftt__DefaultRelease;
	result.userData = ftt_null;
	return result;
}

// ----------------------------------------------------------------------------
// Arena
// ----------------------------------------------------------------------------

ftt_inline size_t ftt__AlignUp(size_t value, size_t alignment) {
	size_t result = (value + (alignment - 1)) & ~(alignment - 1);
	return result;
}

static void ftt__ArenaInit(fttArena *arena, fttAllocator allocator, size_t minBlockSize) {
	arena->first = ftt_null;
	arena->current = ftt_null;
	arena->allocator = allocator;
	arena->minBlockSize = minBlockSize;
	arena->outOfMemory = false;
}

static fttMemoryBlock *ftt__ArenaAddBlock(fttArena *arena, size_t minSize) {
	size_t headerSize = ftt__AlignUp(sizeof(fttMemoryBlock), FTT__ALIGNMENT);
	size_t dataSize = arena->minBlockSize;
	if (minSize > dataSize) {
		dataSize = minSize;
	}
	size_t total = headerSize + dataSize;
	uint8_t *mem = (uint8_t *)arena->allocator.allocate(arena->allocator.userData, total);
	FTT_ASSERT(mem != ftt_null);
	if (mem == ftt_null) {
		// Fail gracefully instead of dereferencing null in release
		arena->outOfMemory = true;
		return ftt_null;
	}
	fttMemoryBlock *block = (fttMemoryBlock *)mem;
	block->base = mem + headerSize;
	block->size = dataSize;
	block->used = 0;
	block->next = ftt_null;
	if (arena->first == ftt_null) {
		arena->first = block;
	} else {
		arena->current->next = block;
	}
	arena->current = block;
	return block;
}

static void *ftt__ArenaPush(fttArena *arena, size_t size) {
	if (size == 0) {
		size = 1;
	}
	fttMemoryBlock *block = arena->current;
	size_t alignedUsed = (block != ftt_null) ? ftt__AlignUp(block->used, FTT__ALIGNMENT) : 0;
	if (block == ftt_null || (alignedUsed + size) > block->size) {
		block = ftt__ArenaAddBlock(arena, size);
		if (block == ftt_null) {
			return ftt_null;
		}
		alignedUsed = 0;
	}
	void *result = block->base + alignedUsed;
	block->used = alignedUsed + size;
	return result;
}

static void ftt__ArenaFree(fttArena *arena) {
	fttMemoryBlock *block = arena->first;
	while (block != ftt_null) {
		fttMemoryBlock *next = block->next;
		arena->allocator.release(arena->allocator.userData, block);
		block = next;
	}
	arena->first = ftt_null;
	arena->current = ftt_null;
}

// ----------------------------------------------------------------------------
// Dynamic array
// ----------------------------------------------------------------------------

static void ftt__ArrayInit(fttArray *a, fttArena *arena, size_t itemSize) {
	a->items = ftt_null;
	a->count = 0;
	a->capacity = 0;
	a->itemSize = itemSize;
	a->arena = arena;
}

static void ftt__ArrayReserve(fttArray *a, size_t n) {
	if (n <= a->capacity) {
		return;
	}
	size_t newCap = (a->capacity > 0) ? (a->capacity * 2) : 8;
	if (newCap < n) {
		newCap = n;
	}
	void *newItems = ftt__ArenaPush(a->arena, newCap * a->itemSize);
	if (newItems == ftt_null) {
		// Out of memory: leave the array as-is so callers keep a valid (if too-small) buffer
		return;
	}
	if (a->count > 0 && a->items != ftt_null) {
		FTT_MEMCPY(newItems, a->items, a->count * a->itemSize);
	}
	a->items = newItems;
	a->capacity = newCap;
}

//! Reserves room for one more element and returns a pointer to the new (uninitialized) slot
static void *ftt__ArrayPush(fttArray *a) {
	ftt__ArrayReserve(a, a->count + 1);
	if (a->count + 1 > a->capacity) {
		// Reserve failed (out of memory); return a valid scratch slot so the immediate write does not crash.
		// The element is discarded and arena->outOfMemory lets the tracer stop.
		FTT_ASSERT(a->itemSize <= sizeof(a->arena->oomSink));
		return a->arena->oomSink;
	}
	void *slot = (uint8_t *)a->items + a->count * a->itemSize;
	a->count++;
	return slot;
}

//! Removes an element while keeping the order of the remaining elements
static void ftt__ArrayRemoveOrdered(fttArray *a, size_t index) {
	FTT_ASSERT(index < a->count);
	uint8_t *base = (uint8_t *)a->items;
	size_t tail = a->count - index - 1;
	if (tail > 0) {
		FTT_MEMMOVE(base + index * a->itemSize, base + (index + 1) * a->itemSize, tail * a->itemSize);
	}
	a->count--;
}

static void ftt__ArrayPop(fttArray *a) {
	FTT_ASSERT(a->count > 0);
	a->count--;
}

static void ftt__ArrayClear(fttArray *a) {
	a->count = 0;
}

//! Pushes a value of the given type into a fttArray
#define FTT__ARRAY_PUSH(a, T, value) (*(T *)ftt__ArrayPush(a) = (value))

// ----------------------------------------------------------------------------
// Math helpers
// ----------------------------------------------------------------------------

ftt_inline fttVec2i ftt__V2i(int32_t x, int32_t y) {
	fttVec2i result;
	result.x = x;
	result.y = y;
	return result;
}

ftt_inline bool ftt__IsEqual(fttVec2i a, fttVec2i b) {
	bool result = (a.x == b.x) && (a.y == b.y);
	return result;
}

ftt_inline fttVec2i ftt__Subtract(fttVec2i a, fttVec2i b) {
	fttVec2i result = ftt__V2i(a.x - b.x, a.y - b.y);
	return result;
}

ftt_inline int32_t ftt__Dot(fttVec2i a, fttVec2i b) {
	int32_t result = a.x * b.x + a.y * b.y;
	return result;
}

ftt_inline int32_t ftt__Cross(fttVec2i a, fttVec2i b) {
	int32_t result = a.x * b.y - a.y * b.x;
	return result;
}

// Up, Right, Down, Left
static const fttVec2i FTT__DIRECTIONS[4] = { { { 0, -1 } }, { { 1, 0 } }, { { 0, 1 } }, { { -1, 0 } } };

// ----------------------------------------------------------------------------
// Tile helpers
// ----------------------------------------------------------------------------

ftt_inline fttTile ftt__MakeTile(int32_t x, int32_t y, int32_t isSolid, fttTileType tileType) {
	fttTile result;
	result.x = x;
	result.y = y;
	result.traceDirection = 0;
	result.isSolid = isSolid;
	result.tileType = tileType;
	return result;
}

ftt_inline fttEdge ftt__MakeTileEdge(int32_t index, int32_t vertIndex0, int32_t vertIndex1, fttVec2i tilePosition) {
	fttEdge result;
	result.index = index;
	result.vertIndex0 = vertIndex0;
	result.vertIndex1 = vertIndex1;
	result.tilePosition = tilePosition;
	result.isInvalid = false;
	return result;
}

ftt_inline uint32_t ftt__ComputeTileIndex(fttVec2u dimension, uint32_t x, uint32_t y) {
	uint32_t result = y * dimension.w + x;
	return result;
}

static int32_t ftt__IsTileSolid(fttArray *tiles, fttVec2u dimension, int32_t x, int32_t y) {
	int32_t result = false;
	if ((x >= 0 && x < (int32_t)dimension.w) && (y >= 0 && y < (int32_t)dimension.h)) {
		uint32_t tileIndex = ftt__ComputeTileIndex(dimension, (uint32_t)x, (uint32_t)y);
		FTT_ASSERT(tileIndex < dimension.w * dimension.h);
		result = FTT_ARRAY_AT(*tiles, fttTile, tileIndex).isSolid > 0;
	}
	return result;
}

static fttTile *ftt__GetTilePtr(fttArray *tiles, fttVec2u dimension, uint32_t x, uint32_t y) {
	FTT_ASSERT((x < dimension.w) && (y < dimension.h));
	uint32_t tileIndex = ftt__ComputeTileIndex(dimension, x, y);
	return FTT_ARRAY_PTR(*tiles, fttTile, tileIndex);
}

static void ftt__SetTileSolid(fttArray *tiles, fttVec2u dimension, uint32_t x, uint32_t y, int32_t value) {
	FTT_ASSERT((x < dimension.w) && (y < dimension.h));
	uint32_t tileIndex = ftt__ComputeTileIndex(dimension, x, y);
	FTT_ARRAY_PTR(*tiles, fttTile, tileIndex)->isSolid = value;
}

static void ftt__RemoveTile(fttArray *tiles, fttVec2u dimension, uint32_t x, uint32_t y) {
	ftt__SetTileSolid(tiles, dimension, x, y, -1);
}

static fttTile *ftt__GetFirstSolidTile(fttArray *tiles, fttVec2u dimension) {
	for (uint32_t tileY = 0; tileY < dimension.h; ++tileY) {
		for (uint32_t tileX = 0; tileX < dimension.w; ++tileX) {
			if (ftt__IsTileSolid(tiles, dimension, (int32_t)tileX, (int32_t)tileY)) {
				return ftt__GetTilePtr(tiles, dimension, tileX, tileY);
			}
		}
	}
	return ftt_null;
}

// Returns which of the four tile corners a slope removes, or -1 to keep all four (solid tile).
// Corner indices follow the clockwise order: 0 = bottom-left, 1 = top-left, 2 = top-right, 3 = bottom-right.
// A slope is simply the full cell with the corner opposite its right angle removed, which turns the two
// edges meeting at that corner into a single hypotenuse while keeping the winding of all other edges intact.
static int32_t ftt__GetSlopeRemovedCorner(fttTileType tileType) {
	switch (tileType) {
		case FTT_TILE_SLOPE_BOTTOM_RIGHT: return 1; // Cuts the top-left corner
		case FTT_TILE_SLOPE_BOTTOM_LEFT:  return 2; // Cuts the top-right corner
		case FTT_TILE_SLOPE_TOP_LEFT:     return 3; // Cuts the bottom-right corner
		case FTT_TILE_SLOPE_TOP_RIGHT:    return 0; // Cuts the bottom-left corner
		default:                          return -1; // Solid tile keeps all four corners
	}
}

ftt_api uint32_t fttGetTileShapeCorners(fttTileType tileType, int32_t x, int32_t y, fttVec2i outCorners[4]) {
	// An empty tile has no shape
	if (tileType == FTT_TILE_EMPTY) {
		return 0;
	}

	// The four cell corners in clockwise order (screen space, y-down)
	fttVec2i corners[4];
	corners[0] = ftt__V2i(x, y + 1);     // Bottom-left
	corners[1] = ftt__V2i(x, y);         // Top-left
	corners[2] = ftt__V2i(x + 1, y);     // Top-right
	corners[3] = ftt__V2i(x + 1, y + 1); // Bottom-right

	int32_t removedCorner = ftt__GetSlopeRemovedCorner(tileType);

	uint32_t count = 0;
	for (int32_t cornerIndex = 0; cornerIndex < 4; ++cornerIndex) {
		if (cornerIndex == removedCorner) {
			continue;
		}
		outCorners[count++] = corners[cornerIndex];
	}
	return count;
}

static fttTileVertices ftt__CreateTileVertices(fttTile *tile) {
	fttTileVertices result;
	result.count = fttGetTileShapeCorners(tile->tileType, tile->x, tile->y, result.verts);
	return result;
}

static fttTileIndices ftt__PushTileVertices(fttTileTracer *tracer, fttTile *tile) {
	fttTileIndices result;
	fttTileVertices tileVerts = ftt__CreateTileVertices(tile);
	result.count = tileVerts.count;
	for (uint32_t vertIndex = 0; vertIndex < tileVerts.count; ++vertIndex) {
		fttVec2i vertex = tileVerts.verts[vertIndex];
		int32_t matchedMainVertexIndex = -1;
		for (uint32_t mainVertexIndex = 0; mainVertexIndex < tracer->mainVertices.count; ++mainVertexIndex) {
			if (ftt__IsEqual(FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, mainVertexIndex), vertex)) {
				matchedMainVertexIndex = (int32_t)mainVertexIndex;
				break;
			}
		}
		if (matchedMainVertexIndex == -1) {
			result.indices[vertIndex] = (int32_t)tracer->mainVertices.count;
			FTT__ARRAY_PUSH(&tracer->mainVertices, fttVec2i, vertex);
		} else {
			result.indices[vertIndex] = matchedMainVertexIndex;
		}
	}
	return result;
}

static fttTileEdges ftt__CreateTileEdges(fttTileIndices tileIndices, fttTile *tile) {
	fttTileEdges result;
	result.count = 0;
	uint32_t indexCount = tileIndices.count;
	for (uint32_t index = 0; index < indexCount; ++index) {
		int32_t e0 = tileIndices.indices[index];
		int32_t e1 = tileIndices.indices[(index + 1) % indexCount];
		result.edges[result.count++] = ftt__MakeTileEdge((int32_t)index, e0, e1, ftt__V2i(tile->x, tile->y));
	}
	return result;
}

static fttTileEdges ftt__RemoveOverlapEdges(fttTileTracer *tracer, fttTileEdges inputEdges) {
	fttTileEdges result;
	result.count = 0;
	for (uint32_t edgeIndex = 0; edgeIndex < inputEdges.count; ++edgeIndex) {
		fttEdge inputEdge = inputEdges.edges[edgeIndex];
		bool addIt = true;
		for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < tracer->mainEdges.count; ++mainEdgeIndex) {
			fttEdge mainEdge = FTT_ARRAY_AT(tracer->mainEdges, fttEdge, mainEdgeIndex);
			if (inputEdge.vertIndex0 == mainEdge.vertIndex1 && inputEdge.vertIndex1 == mainEdge.vertIndex0) {
				addIt = false;
				ftt__ArrayRemoveOrdered(&tracer->mainEdges, mainEdgeIndex);
				break;
			}
		}
		if (addIt) {
			result.edges[result.count++] = inputEdge;
		}
	}
	return result;
}

static bool ftt__IsTileSharesCommonEdges(fttTileTracer *tracer, fttTileVertices tileVertices) {
	uint32_t vertexCount = tileVertices.count;
	for (uint32_t vertIndex = 0; vertIndex < vertexCount; ++vertIndex) {
		fttVec2i tv0 = tileVertices.verts[vertIndex];
		fttVec2i tv1 = tileVertices.verts[(vertIndex + 1) % vertexCount];
		for (uint32_t edgeIndex = 0; edgeIndex < tracer->mainEdges.count; ++edgeIndex) {
			fttEdge edge = FTT_ARRAY_AT(tracer->mainEdges, fttEdge, edgeIndex);
			fttVec2i mv0 = FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, edge.vertIndex0);
			fttVec2i mv1 = FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, edge.vertIndex1);
			if (ftt__IsEqual(tv0, mv1) && ftt__IsEqual(tv1, mv0)) {
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------------
// Chain segment helpers
// ----------------------------------------------------------------------------

static void ftt__RemoveSegmentVertex(fttChainSegment *chainSegment, uint32_t index) {
	FTT_ASSERT(index < chainSegment->vertices.count);
	ftt__ArrayRemoveOrdered(&chainSegment->vertices, index);
}

static void ftt__ClearLineSegmentPoints(fttChainSegment *segment, uint32_t firstIndex, uint32_t middleIndex, uint32_t lastIndex) {
	fttVec2i last = FTT_ARRAY_AT(segment->vertices, fttVec2i, lastIndex);
	fttVec2i middle = FTT_ARRAY_AT(segment->vertices, fttVec2i, middleIndex);
	fttVec2i first = FTT_ARRAY_AT(segment->vertices, fttVec2i, firstIndex);
	fttVec2i d1 = ftt__Subtract(last, middle);
	fttVec2i d2 = ftt__Subtract(middle, first);
	// Only remove the middle vertex when it lies exactly on the straight line between its neighbours.
	// The cross product tests collinearity; the dot product guards against a degenerate reversal.
	// A plain dot-product test would wrongly collapse genuine slope corners, because a diagonal
	// hypotenuse meets an axis-aligned edge at a shallow angle that still has a positive dot product.
	int32_t cross = ftt__Cross(d1, d2);
	int32_t dot = ftt__Dot(d1, d2);
	if (cross == 0 && dot > 0) {
		ftt__RemoveSegmentVertex(segment, middleIndex);
	}
}

static void ftt__OptimizeChainSegment(fttChainSegment *segment) {
	if (segment->vertices.count > 2) {
		uint32_t lastIndex = (uint32_t)(segment->vertices.count - 1);
		ftt__ClearLineSegmentPoints(segment, lastIndex - 2, lastIndex - 1, lastIndex);
	}
}

static void ftt__FinalizeChainSegment(fttChainSegment *chainSegment) {
	if (chainSegment->vertices.count > 2) {
		ftt__ClearLineSegmentPoints(chainSegment, (uint32_t)(chainSegment->vertices.count - 1), 0, 1);
	}
	if (chainSegment->vertices.count > 2) {
		ftt__ClearLineSegmentPoints(chainSegment, 0, (uint32_t)(chainSegment->vertices.count - 1), (uint32_t)(chainSegment->vertices.count - 2));
	}
}

static void ftt__AddChainSegmentVertex(fttChainSegment *chainSegment, fttVec2i vertex) {
	FTT__ARRAY_PUSH(&chainSegment->vertices, fttVec2i, vertex);
}

// ----------------------------------------------------------------------------
// State machine helpers
// ----------------------------------------------------------------------------

//! Returns the chain segment currently being built (or null)
static fttChainSegment *ftt__CurChainSegment(fttTileTracer *tracer) {
	if (tracer->curChainSegmentIndex < 0) {
		return ftt_null;
	}
	return FTT_ARRAY_PTR(tracer->chainSegments, fttChainSegment, tracer->curChainSegmentIndex);
}

static bool ftt__ProcessTraverseNextEdge(fttTileTracer *tracer) {
	fttEdge *lastEdge = FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, tracer->lastEdgeIndex);
	fttEdge *startEdge = FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, tracer->startEdgeIndex);
	for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < tracer->mainEdges.count; ++mainEdgeIndex) {
		fttEdge *curEdge = FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, mainEdgeIndex);
		if (!curEdge->isInvalid && (curEdge->vertIndex0 == lastEdge->vertIndex1)) {
			fttChainSegment *curChainSegment = ftt__CurChainSegment(tracer);
			// If v0 from current edge equals starting edge - then we are finished
			if (curEdge->vertIndex1 == startEdge->vertIndex0) {
				// We are done with this line segment - Set cur step to find next starting edge
				tracer->lastEdgeIndex = -1;
				tracer->curStep = FTT_STEP_TRAVERSE_FIND_STARTING_EDGE;
				// Optimize and finalize shape
				ftt__OptimizeChainSegment(curChainSegment);
				ftt__FinalizeChainSegment(curChainSegment);
				// Add first vertex to the end again, because we have a fully closed chain
				ftt__AddChainSegmentVertex(curChainSegment, FTT_ARRAY_AT(curChainSegment->vertices, fttVec2i, 0));
			} else {
				// Now our current edge is the last edge
				tracer->lastEdgeIndex = (int32_t)mainEdgeIndex;
				// Add always the first edge vertex to the list
				ftt__AddChainSegmentVertex(curChainSegment, FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, curEdge->vertIndex1));
				// Optimize shape
				ftt__OptimizeChainSegment(curChainSegment);
			}
			curEdge->isInvalid = true;
			++tracer->consumedEdgeCount;
			return true;
		}
	}

	// We will come here for a line segment which is not fully closed, may have holes or something
	fttChainSegment *curChainSegment = ftt__CurChainSegment(tracer);
	if (curChainSegment != ftt_null && curChainSegment->vertices.count > 0) {
		// We are done with this line segment - Set cur step to find next starting edge
		tracer->lastEdgeIndex = -1;
		tracer->curStep = FTT_STEP_TRAVERSE_FIND_STARTING_EDGE;
		// Optimize and finalize shape
		ftt__OptimizeChainSegment(curChainSegment);
		ftt__FinalizeChainSegment(curChainSegment);
		return true;
	}

	// We are totally done, there is no line segment or anything to do
	tracer->curStep = FTT_STEP_DONE;
	return false;
}

static bool ftt__ProcessTraverseFindStartingEdge(fttTileTracer *tracer) {
	bool result = false;

	// Find next free starting edge - at the start this is always none
	tracer->startEdgeIndex = -1;
	for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < tracer->mainEdges.count; ++mainEdgeIndex) {
		fttEdge *mainEdge = FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, mainEdgeIndex);
		if (!mainEdge->isInvalid) {
			tracer->startEdgeIndex = (int32_t)mainEdgeIndex;
			break;
		}
	}

	if (tracer->startEdgeIndex >= 0) {
		// Continue when we found a starting edge and create the actual line segment for it
		result = true;
		tracer->lastEdgeIndex = tracer->startEdgeIndex;
		fttEdge *startEdge = FTT_ARRAY_PTR(tracer->mainEdges, fttEdge, tracer->startEdgeIndex);
		startEdge->isInvalid = true;
		++tracer->consumedEdgeCount;
		int32_t v0 = startEdge->vertIndex0;
		int32_t v1 = startEdge->vertIndex1;
		tracer->curStep = FTT_STEP_TRAVERSE_NEXT_EDGE;

		// Create a fresh chain segment
		fttChainSegment newSegment;
		ftt__ArrayInit(&newSegment.vertices, &tracer->arena, sizeof(fttVec2i));
		ftt__ArrayReserve(&newSegment.vertices, tracer->mainEdges.count + 1);
		FTT__ARRAY_PUSH(&tracer->chainSegments, fttChainSegment, newSegment);
		tracer->curChainSegmentIndex = (int32_t)(tracer->chainSegments.count - 1);

		fttChainSegment *curChainSegment = ftt__CurChainSegment(tracer);
		ftt__AddChainSegmentVertex(curChainSegment, FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, v0));
		ftt__AddChainSegmentVertex(curChainSegment, FTT_ARRAY_AT(tracer->mainVertices, fttVec2i, v1));
	} else {
		// We are completely done
		tracer->curStep = FTT_STEP_DONE;
	}

	return result;
}

static void ftt__GetNextOpenTile(fttTileTracer *tracer) {
	if (tracer->openList.count > 0) {
		tracer->curTile = FTT_ARRAY_AT(tracer->openList, fttTile *, tracer->openList.count - 1);
		tracer->curStep = FTT_STEP_FIND_NEXT_TILE;
	} else {
		tracer->curStep = FTT_STEP_FIND_START;
	}
}

static void ftt__RotateForward(fttTileTracer *tracer) {
	if (tracer->curTile->traceDirection < (FTT_DIRECTION_COUNT - 1)) {
		++tracer->curTile->traceDirection;
		tracer->curStep = FTT_STEP_FIND_NEXT_TILE;
	} else {
		ftt__ArrayPop(&tracer->openList);
		tracer->curStep = FTT_STEP_GET_NEXT_OPEN_TILE;
		ftt__GetNextOpenTile(tracer);
	}
}

static void ftt__AddTile(fttTileTracer *tracer, fttTile *tile) {
	// Add the tile to the open list and remove it from the map
	FTT__ARRAY_PUSH(&tracer->openList, fttTile *, tile);
	ftt__RemoveTile(&tracer->tiles, tracer->tileCount, (uint32_t)tile->x, (uint32_t)tile->y);
	++tracer->processedTileCount;

	// Create tile vertices/indices and edges for the tile
	fttTileIndices tileIndices = ftt__PushTileVertices(tracer, tile);
	fttTileEdges tileEdges = ftt__CreateTileEdges(tileIndices, tile);

	// Remove edges that overlap from the main edge list and the edge list for this tile
	tileEdges = ftt__RemoveOverlapEdges(tracer, tileEdges);

	// Push the remaining edges to the main edges list
	for (uint32_t tileEdgeIndex = 0; tileEdgeIndex < tileEdges.count; ++tileEdgeIndex) {
		FTT__ARRAY_PUSH(&tracer->mainEdges, fttEdge, tileEdges.edges[tileEdgeIndex]);
	}
}

// ----------------------------------------------------------------------------
// Public api
// ----------------------------------------------------------------------------

ftt_api void fttInitTileTracer(fttTileTracer *tracer, fttVec2u tileCount, const uint8_t *mapTiles, const fttAllocator *allocator) {
	FTT_ASSERT(tracer != ftt_null);
	FTT_ASSERT(mapTiles != ftt_null);

	tracer->allocator = (allocator != ftt_null) ? *allocator : fttDefaultAllocator();
	ftt__ArenaInit(&tracer->arena, tracer->allocator, FTT_ARENA_MIN_BLOCK_SIZE);

	tracer->tileCount = tileCount;

	uint32_t tileTotal = tileCount.w * tileCount.h;

	// Initialize all the dynamic arrays and pre-reserve based on the known bounds
	ftt__ArrayInit(&tracer->tiles, &tracer->arena, sizeof(fttTile));
	ftt__ArrayInit(&tracer->openList, &tracer->arena, sizeof(fttTile *));
	ftt__ArrayInit(&tracer->mainVertices, &tracer->arena, sizeof(fttVec2i));
	ftt__ArrayInit(&tracer->mainEdges, &tracer->arena, sizeof(fttEdge));
	ftt__ArrayInit(&tracer->chainSegments, &tracer->arena, sizeof(fttChainSegment));

	ftt__ArrayReserve(&tracer->tiles, tileTotal);
	ftt__ArrayReserve(&tracer->openList, tileTotal);
	ftt__ArrayReserve(&tracer->mainVertices, tileTotal * 4);
	ftt__ArrayReserve(&tracer->mainEdges, tileTotal * 4);
	ftt__ArrayReserve(&tracer->chainSegments, 64);

	// If the tile buffer could not be reserved (out of memory), leave the tracer in a valid, empty, done state
	// instead of writing tiles into a null buffer below.
	if (tracer->arena.outOfMemory || tracer->tiles.capacity < tileTotal) {
		tracer->tiles.count = 0;
		tracer->startTile = ftt_null;
		tracer->curTile = ftt_null;
		tracer->nextTile = ftt_null;
		tracer->startEdgeIndex = -1;
		tracer->lastEdgeIndex = -1;
		tracer->curChainSegmentIndex = -1;
		tracer->totalSolidTileCount = 0;
		tracer->processedTileCount = 0;
		tracer->totalEdgeCount = 0;
		tracer->consumedEdgeCount = 0;
		tracer->curStep = FTT_STEP_DONE;
		return;
	}

	// Fill the tile array from the map and count how many tiles are solid
	tracer->tiles.count = tileTotal;
	uint32_t solidTileCount = 0;
	for (uint32_t tileY = 0; tileY < tileCount.h; ++tileY) {
		for (uint32_t tileX = 0; tileX < tileCount.w; ++tileX) {
			uint32_t tileIndex = ftt__ComputeTileIndex(tileCount, tileX, tileY);
			fttTileType tileType = (fttTileType)mapTiles[tileIndex];
			int32_t isSolid = (int32_t)mapTiles[tileIndex];
			if (isSolid > 0) {
				++solidTileCount;
			}
			FTT_ARRAY_AT(tracer->tiles, fttTile, tileIndex) = ftt__MakeTile((int32_t)tileX, (int32_t)tileY, isSolid, tileType);
		}
	}

	tracer->curStep = FTT_STEP_FIND_START;
	tracer->startTile = ftt_null;
	tracer->curTile = ftt_null;
	tracer->nextTile = ftt_null;
	tracer->startEdgeIndex = -1;
	tracer->lastEdgeIndex = -1;
	tracer->curChainSegmentIndex = -1;

	tracer->totalSolidTileCount = solidTileCount;
	tracer->processedTileCount = 0;
	tracer->totalEdgeCount = 0;
	tracer->consumedEdgeCount = 0;
}

ftt_api bool fttNextTileTraceStep(fttTileTracer *tracer) {
	FTT_ASSERT(tracer != ftt_null);

	// Stop stepping if the arena ran out of memory (a prior push was discarded); output is incomplete but no crash
	if (tracer->arena.outOfMemory) {
		tracer->curStep = FTT_STEP_DONE;
		return false;
	}

	bool result = true;
	switch (tracer->curStep) {
		case FTT_STEP_DONE:
		{
			result = false;
		} break;

		case FTT_STEP_TRAVERSE_NEXT_EDGE:
		{
			result = ftt__ProcessTraverseNextEdge(tracer);
		} break;

		case FTT_STEP_TRAVERSE_FIND_STARTING_EDGE:
		{
			result = ftt__ProcessTraverseFindStartingEdge(tracer);
		} break;

		case FTT_STEP_FIND_START:
		{
			ftt__ArrayClear(&tracer->openList);
			tracer->curTile = ftt_null;
			tracer->startTile = ftt__GetFirstSolidTile(&tracer->tiles, tracer->tileCount);
			if (tracer->startTile != ftt_null) {
				// Add the start tile to the open list and build vertices and edges from it
				ftt__AddTile(tracer, tracer->startTile);

				// Set next step to get open tile and process it immediately
				tracer->curStep = FTT_STEP_GET_NEXT_OPEN_TILE;
				ftt__GetNextOpenTile(tracer);
			} else {
				// No start found, exit if we have not found any edges at all
				if (tracer->mainEdges.count == 0) {
					result = false;
				} else {
					// Clear all chain segments and switch to phase 2
					ftt__ArrayClear(&tracer->chainSegments);
					tracer->curChainSegmentIndex = -1;
					// All boundary edges are now known - remember them as the phase 2 denominator
					tracer->totalEdgeCount = (uint32_t)tracer->mainEdges.count;
					tracer->curStep = FTT_STEP_TRAVERSE_FIND_STARTING_EDGE;
				}
			}
		} break;

		case FTT_STEP_GET_NEXT_OPEN_TILE:
		{
			ftt__GetNextOpenTile(tracer);
		} break;

		case FTT_STEP_FIND_NEXT_TILE:
		{
			FTT_ASSERT(tracer->curTile != ftt_null);

			// Tile in the "forward" direction of the current tile
			int32_t nx = tracer->curTile->x + FTT__DIRECTIONS[tracer->curTile->traceDirection].x;
			int32_t ny = tracer->curTile->y + FTT__DIRECTIONS[tracer->curTile->traceDirection].y;
			tracer->nextTile = ftt__IsTileSolid(&tracer->tiles, tracer->tileCount, nx, ny) ? ftt__GetTilePtr(&tracer->tiles, tracer->tileCount, (uint32_t)nx, (uint32_t)ny) : ftt_null;

			if (tracer->nextTile != ftt_null) {
				// Create tile vertices for next tile and check if it shares common edges
				fttTileVertices tileVertices = ftt__CreateTileVertices(tracer->nextTile);
				bool sharesCommonEdge = ftt__IsTileSharesCommonEdges(tracer, tileVertices);
				if (sharesCommonEdge) {
					// Add the next tile to the open list and build vertices and edges from it
					ftt__AddTile(tracer, tracer->nextTile);

					// Set next step to get open tile and process it immediately
					tracer->curStep = FTT_STEP_GET_NEXT_OPEN_TILE;
					ftt__GetNextOpenTile(tracer);
				} else {
					tracer->curStep = FTT_STEP_ROTATE_FORWARD;
					ftt__RotateForward(tracer);
				}
			} else {
				tracer->curStep = FTT_STEP_ROTATE_FORWARD;
				ftt__RotateForward(tracer);
			}
		} break;

		case FTT_STEP_ROTATE_FORWARD:
		{
			ftt__RotateForward(tracer);
		} break;

		default:
			FTT_ASSERT(!"Invalid default case!");
	}
	return result;
}

ftt_api void fttRunTileTracer(fttTileTracer *tracer) {
	FTT_ASSERT(tracer != ftt_null);
	while (fttNextTileTraceStep(tracer)) {
	}
}

ftt_api void fttFreeTileTracer(fttTileTracer *tracer) {
	FTT_ASSERT(tracer != ftt_null);
	ftt__ArenaFree(&tracer->arena);

	// Reset to a safe empty state
	tracer->tiles.items = ftt_null;
	tracer->tiles.count = 0;
	tracer->tiles.capacity = 0;
	tracer->openList.items = ftt_null;
	tracer->openList.count = 0;
	tracer->openList.capacity = 0;
	tracer->mainVertices.items = ftt_null;
	tracer->mainVertices.count = 0;
	tracer->mainVertices.capacity = 0;
	tracer->mainEdges.items = ftt_null;
	tracer->mainEdges.count = 0;
	tracer->mainEdges.capacity = 0;
	tracer->chainSegments.items = ftt_null;
	tracer->chainSegments.count = 0;
	tracer->chainSegments.capacity = 0;
	tracer->startTile = ftt_null;
	tracer->curTile = ftt_null;
	tracer->nextTile = ftt_null;
	tracer->startEdgeIndex = -1;
	tracer->lastEdgeIndex = -1;
	tracer->curChainSegmentIndex = -1;
	tracer->curStep = FTT_STEP_DONE;
	tracer->totalSolidTileCount = 0;
	tracer->processedTileCount = 0;
	tracer->totalEdgeCount = 0;
	tracer->consumedEdgeCount = 0;
}

#ifdef __cplusplus
}
#endif

#endif // FTT_IMPLEMENTATION
