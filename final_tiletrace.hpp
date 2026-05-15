/**
* @file final_tiletrace.hpp
* @version v1.02
* @author Torsten Spaete
* @brief Final TileTrace (FTT) - an open source single file header C++ contour tile tracing library.
*
* This creates chainshapes out of any solid tilemap based on a contour tracing algorithm.
*
* @mainpage
* Summary of the Final TileTrace (FTT) project.
* Please see @ref final_tiletrace.hpp for more details.
*/

/*
final_tiletrace.hpp
Open-Source Single-File Header C++ Library by Torsten Spaete

This creates chainshapes out of any solid tilemap based on a contour tracing algorithm.
It will try to create as few chain shapes as possible.

- Supports block tiles only!

# Requirements

- STL (Standard Template Library)

# HOW TO USE

# EXAMPLES

# PREPROCESSOR OVERRIDES

# ALGORITHM

## Overview

The algorithm converts a 2D grid of solid tiles into a minimal set of closed (or
open) chain segments — polylines that trace the contours of all solid regions.
It operates in two distinct phases driven by an explicit state machine (Steps::StepEnum):

  Phase 1  — Tile Expansion: expand through the tilemap using a contour-aware
             flood-fill to build a global edge mesh containing only the *boundary*
             edges of all solid regions.  Interior edges shared by two adjacent
             solid tiles cancel each other out via antiparallel edge removal.

  Phase 2  — Edge Traversal: walk the edge mesh in order to extract closed (or
             open) chain segments, suitable for use as collision shapes
             (e.g. Box2D b2ChainShape).

Both phases can be advanced one step at a time via NextTileTraceStep(), or run
to completion with RunTileTracer().

---------------------------------------------------------------------------
## Tile Coordinate System and Vertex Layout

Tiles are addressed by (tileX, tileY) with (0,0) at the top-left corner.
Each tile occupies one unit in world space.  The four corner vertices of a
tile at position (x, y) are generated as:

    v1(x,   y  ) ----[e1]---- v2(x+1, y  )
         |                          |
        [e0]                       [e2]
         |                          |
    v0(x,   y+1) ----[e3]---- v3(x+1, y+1)

  v0 = bottom-left  = (x,   y+1)
  v1 = top-left     = (x,   y  )
  v2 = top-right    = (x+1, y  )
  v3 = bottom-right = (x+1, y+1)

Edges are directed *clockwise* (outer normal faces outward for a solid tile):
  e0 (left  ): v0 -> v1
  e1 (top   ): v1 -> v2
  e2 (right ): v2 -> v3
  e3 (bottom): v3 -> v0

---------------------------------------------------------------------------
## Phase 1 — Tile Expansion (Building the Boundary Edge Mesh)

Goal: populate mainEdges with every edge that lies on the boundary between a
solid tile and empty space.  Interior edges shared by two adjacent solid tiles
cancel each other out.

### Step: FindStart
  Scan the tile array in row-major order (top-left to bottom-right) for the
  first tile whose isSolid > 0.  This becomes startTile.  Call AddTile on it,
  then immediately transition to GetNextOpenTile.
  If no solid tile is found and mainEdges is non-empty, skip straight to
  TraverseFindStartingEdge (Phase 2).  If mainEdges is also empty, stop (Done).

### AddTile (helper called whenever a new solid tile is accepted)
  1. Push the tile onto the openList (used as a LIFO stack).
  2. Mark the tile as visited by setting isSolid = -1 (RemoveTile), so it is
     never returned by FindStart or FindNextTile again.
  3. Generate or reuse the four corner vertices in mainVertices.
     PushTileVertices deduplicates: a vertex is only appended to mainVertices if
     no existing entry matches it exactly.  Returns TileIndices holding the four
     indices into mainVertices for this tile's corners.
  4. Build four directed, clockwise edges (CreateTileEdges) from the TileIndices.
  5. Cancel shared interior edges (RemoveOverlapEdges):
     An input edge (v0->v1) cancels an existing main edge when the main edge
     runs in the exact opposite direction (mainEdge.v0 == v1 && mainEdge.v1 == v0).
     Both the input edge and the matching main edge are discarded.  This removes
     the shared wall between two adjacent solid tiles.
  6. Append all surviving (boundary) edges to mainEdges.

### Step: GetNextOpenTile
  Peek at the last element of openList (LIFO) and set it as curTile, then
  transition to FindNextTile.  If openList is empty, go back to FindStart to
  look for the next disconnected solid region.

### Step: FindNextTile
  Look at the tile one step ahead of curTile in its current traceDirection
  (Up=0, Right=1, Down=2, Left=3).  Accept the neighbour tile if and only if:
    * it exists within map bounds and has isSolid > 0, AND
    * it shares at least one common edge with the current mainEdges set
      (IsTileSharesCommonEdges — checks that one of the candidate tile's four
      potential edges runs antiparallel to an existing main edge).
  If accepted: call AddTile on it and transition to GetNextOpenTile.
  Otherwise: transition to RotateForward.

  Note: this is NOT a simple 4-neighbour flood fill.  A neighbour is only
  accepted when it touches the *existing edge mesh*, keeping expansion coherent
  along the contour rather than filling the entire solid interior.

### Step: RotateForward
  Advance curTile->traceDirection by one (Up->Right->Down->Left).
  If all four directions have been tried (direction would exceed Left), pop
  curTile from openList and call GetNextOpenTile to process the next tile.
  This guarantees every reachable solid neighbour is eventually visited.

---------------------------------------------------------------------------
## Phase 2 — Edge Traversal (Building Chain Segments)

Once Phase 1 is complete, mainEdges contains all boundary edges.  Phase 2
traverses them to form ordered polylines.

### Step: TraverseFindStartingEdge
  Find the first edge in mainEdges whose isInvalid flag is false.  Mark it
  invalid (consumed), create a new ChainSegment, and add both its endpoints
  as the first two vertices.  Set startEdge and lastEdge to this edge, then
  transition to TraverseNextEdge.
  If no free edge exists, transition to Done.

### Step: TraverseNextEdge
  Find the next edge whose vertIndex0 == lastEdge->vertIndex1 (i.e. the edge
  that begins where the last consumed edge ends).  Mark it invalid.

  Two sub-cases:
  a) Closed loop: if the found edge's vertIndex1 == startEdge->vertIndex0, the
     contour has returned to its origin.  Optimize (remove collinear vertices),
     finalize (check wrap-around seam), then duplicate the first vertex at the
     end (closed chain convention).  Return to TraverseFindStartingEdge for the
     next independent contour.
  b) Open continuation: append the edge's endpoint to the segment, call
     OptimizeChainSegment, update lastEdge, and loop.

  If no matching continuation edge exists, the contour is open (e.g. a boundary
  touching the map edge).  Finalize what has been built and return to
  TraverseFindStartingEdge.

---------------------------------------------------------------------------
## Collinearity Optimization (ClearLineSegmentPoints)

After each vertex is appended, OptimizeChainSegment checks the last three
vertices (first, middle, last).  Given:
  d1 = last   - middle
  d2 = middle - first
If dot(d1, d2) > 0 (both vectors point in the same general direction), the
three points are collinear and the middle vertex is redundant — it is removed.

FinalizeChainSegment applies the same test at the wrap-around seam:
  (last-2, last-1, last) and (last, first, second)
to clean up collinear vertices that span the segment boundary.

---------------------------------------------------------------------------
## Output

After the algorithm completes (NextTileTraceStep returns false):
  chainSegments  — one ChainSegment per connected contour boundary.  Each
                   closed segment ends with a copy of its first vertex.
                   Collinear intermediate vertices have been removed.
  mainVertices   — all unique corner vertices referenced by the segments.
  mainEdges      — all boundary edges (all marked isInvalid = true after
                   traversal).

# FEATURES

[X] Block tile contour tracing
[X] Creating optimized chain segments

# TODO

[ ] Finish algorithm documentation

# LICENSE

MIT License

Copyright (c) 2017-2019 Torsten Spaete
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

# VERSION HISTORY

- v1.01:
	* Added additional C++ api
- v1.0:
	* Initial version

*/
#ifndef FTT_INCLUDE_HPP
#define FTT_INCLUDE_HPP

#include <vector>
#include <inttypes.h>

#if !defined(FTT_API_AS_PRIVATE)
#	define FTT_API_AS_PRIVATE 0
#endif

#if FTT_API_AS_PRIVATE
#	define ftt_api static
#else
#	define ftt_api extern
#endif // FTT_API_AS_PRIVATE

//! Null pointer
#if (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
#	define ftt_null nullptr
#else
#	define ftt_null 0
#endif

//! Core namespace for final tiletracing
namespace ftt {
	//! 2D signed 32-bit integer vector
	union Vec2i {
		struct {
			int32_t x;
			int32_t y;
		};
		struct {
			int32_t w;
			int32_t h;
		};
		int32_t e[2];
	};

	//! 2D unsigned 32-bit integer vector
	union Vec2u {
		struct {
			uint32_t x;
			uint32_t y;
		};
		struct {
			uint32_t w;
			uint32_t h;
		};
		uint32_t e[2];
	};

	//! Search directions
	namespace Directions {
		//! Search direction
		enum DirectionEnum {
			Up,
			Right,
			Down,
			Left,
		};
	};
	typedef Directions::DirectionEnum Direction;

	//! Trace steps (FindStart, GetNextOpenTile, RotateForward, etc.)
	namespace Steps {
		//! Trace step
		enum StepEnum {
			None,
			FindStart,
			GetNextOpenTile,
			FindNextTile,
			RotateForward,
			TraverseFindStartingEdge,
			TraverseNextEdge,
			Done,
		};
	};
	typedef Steps::StepEnum Step;

	//! A single tile in the map.  isSolid > 0 means solid, 0 means empty,
	//! -1 means already visited/removed by the tracer.
	//! traceDirection indexes into TILETRACE_DIRECTIONS (0=Up,1=Right,2=Down,3=Left)
	//! and tracks which direction the tracer is currently scanning from this tile.
	struct Tile {
		int32_t x, y;            //!< Tile grid coordinates
		uint32_t traceDirection; //!< Current scan direction index (0-3)
		int32_t isSolid;         //!< >0 solid, 0 empty, -1 visited/removed
	};

	//! A directed boundary edge between two vertices in mainVertices.
	//! Edges are wound clockwise around a solid tile (outer normal faces outward).
	//! Two edges with swapped indices (v0==other.v1 && v1==other.v0) are antiparallel
	//! and represent a shared interior wall — they cancel each other during Phase 1.
	struct Edge {
		int32_t index;              //!< Local edge index within the tile (0-3: left, top, right, bottom)
		int32_t vertIndex0;         //!< Start vertex index into TileTracerData::mainVertices
		int32_t vertIndex1;         //!< End   vertex index into TileTracerData::mainVertices
		Vec2i tilePosition;         //!< Grid position of the tile this edge originated from
		bool isInvalid;             //!< True once this edge has been consumed by Phase 2 traversal
	};

	//! The four world-space corner positions for a single tile (see vertex layout in ALGORYTHM).
	//! verts[0]=bottom-left, verts[1]=top-left, verts[2]=top-right, verts[3]=bottom-right.
	struct TileVertices {
		Vec2i verts[4];
	};

	//! Indices into TileTracerData::mainVertices for each of a tile's four corners.
	//! Parallel to TileVertices: indices[i] is the mainVertices index for verts[i].
	struct TileIndices {
		int32_t indices[4];
	};

	//! Up to four directed edges produced for a single tile, with the count of valid entries.
	//! After RemoveOverlapEdges the count may be less than 4 (interior edges are discarded).
	struct TileEdges {
		Edge edges[4];   //!< Edge array (only entries [0..count-1] are valid)
		uint32_t count;  //!< Number of valid edges in the array
	};

	//! A single output contour — an ordered list of world-space vertices.
	//! Closed contours have vertices.front() == vertices.back().
	//! Collinear intermediate vertices are removed by the optimizer.
	struct ChainSegment {
		std::vector<Vec2i> vertices;
	};

	//! Full mutable state of the tile tracer.  Holds the tile grid, the growing
	//! vertex/edge mesh built during Phase 1, the open-list stack used for
	//! expansion, and the chain segments produced during Phase 2.
	struct TileTracerData {
		Vec2u tileCount;                   //!< Width and height of the tile grid
		std::vector<Tile> tiles;           //!< Flat row-major tile array (modified in-place; visited tiles become isSolid=-1)
		Step curStep;                      //!< Current state-machine step
		Tile *startTile;                   //!< Pointer to the start tile of the current expansion region
		Tile *curTile;                     //!< Pointer to the tile currently being expanded from
		Tile *nextTile;                    //!< Pointer to the candidate neighbour tile (transient, used in FindNextTile)
		Edge *startEdge;                   //!< Pointer to the first edge of the current chain segment (Phase 2)
		Edge *lastEdge;                    //!< Pointer to the most recently consumed edge (Phase 2)
		ChainSegment *curChainSegment;     //!< Pointer into chainSegments to the segment being built (Phase 2)
		std::vector<Tile *> openList;      //!< LIFO stack of solid tiles waiting to be expanded (Phase 1)
		std::vector<Vec2i> mainVertices;   //!< Deduplicated world-space vertex pool shared across all tiles
		std::vector<Edge> mainEdges;       //!< Boundary edges surviving antiparallel cancellation (Phase 1 output / Phase 2 input)
		std::vector<ChainSegment> chainSegments; //!< Final output: one segment per contiguous solid-region boundary
	};

	//! Tile tracer C++ API
	class TileTracer {
	private:
		TileTracerData data;
	public:
		//! Constructs a tile tracer instance for the given tile map
		TileTracer(const Vec2u &tileCount, uint8_t *mapTiles);

		//! Executes the next step for the tracer
		bool Next();
		//! Runs the full tracer until it is done.
		void Run();

		//! Returns the number of chain segments
		inline size_t GetChainSegmentCount() const {
			return data.chainSegments.size();
		}
		//! Returns a chain segment by the given index
		inline const ChainSegment &GetChainSegment(uint32_t index) const {
			return data.chainSegments[index];
		}
		//! Returns the number of vertices
		inline size_t GetVertexCount() const {
			return data.mainVertices.size();
		}
		//! Returns a vertex by the given index
		inline const Vec2i &GetVertex(uint32_t index) const {
			return data.mainVertices[index];
		}
		//! Returns the number of edges
		inline size_t GetEdgeCount() const {
			return data.mainEdges.size();
		}
		//! Returns an edge by the given index
		inline const Edge &GetEdge(uint32_t index) const {
			return data.mainEdges[index];
		}
		//! Returns a tile by the given coordinates
		inline const Tile &GetTile(uint32_t x, uint32_t y) const {
			return data.tiles[y * data.tileCount.w + x];
		}
		//! Returns the number of open tiles
		inline size_t GetOpenTileCount() const {
			return data.openList.size();
		}
		//! Returns an open tile pointer by the given index
		inline Tile *GetOpenTile(uint32_t index) const {
			return data.openList[index];
		}
		//! Returns the start tile pointer
		inline Tile *GetStartTile() const {
			return data.startTile;
		}
		//! Returns the current tile pointer
		inline Tile *GetCurrentTile() const {
			return data.curTile;
		}
	};

	//! Initializes a tile tracer data so you can start tracing your tilemap
	ftt_api void InitTileTracer(TileTracerData *tracer, const Vec2u &tileCount, uint8_t *mapTiles);
	//! Executes the next step for the given tracer data
	ftt_api bool NextTileTraceStep(TileTracerData *tracer);
	//! Runs the full tracer until it is done for the given tracer data.
	ftt_api void RunTileTracer(TileTracerData *tracer);
};
#endif

#if defined(FTT_IMPLEMENTATION) && !defined(FTT_IMPLEMENTED)
#	define FTT_IMPLEMENTED

#include <assert.h>

namespace ftt {
	/*
	https://en.wikipedia.org/wiki/Moore_neighborhood

	How this algorithm works:

	Find first/next start:
	- Find first solid tile anywhere in the map -> Start-Tile
	- Store this tile as Current-Tile
	- Clear the OpenTile-List

	Add Tile:
	- Mark that Start-Tile as invalid, so we will never process it again
	- Set the initial scan direction for this Start-Tile to Up
	- This Start-Tile is pushed into the OpenTile-List
	- Create all 4 vertices for that tile and push it into the Main-Verts-List, but dont push any vertex which is already in the list
	- Remember each index to the Main-Verts-List you have pushed on or found when already exists
	- Create 4 edges for that tile and store it in a local array with 4 entries long -> Each edge defines the first index and the following index is defined clockwise direction

	A tile and its vertices and edge indices (Winding order is important!):

		   e1
	v1 |--------| v2
	   |        |
	e0 |        | e2
	   |        |
	v0 |--------| v3
		   e3

	- Loop through all 4 edges and remove edges which overlaps any edge in the Main-Edge-List:
	A overlap edge is detected when the follow criteria are met: (MainEdge->Index1 == CurrentEdge->Index0) and (MainEdge->Index0 == CurrentEdge->Index1)
	- Add all non-overlapping edges to the Main-Edge-List
	- Get next open tile from the Open-Tile-List, but leave list untouched and continue on "Get next tile forward"
	or
	When there is no tile in the Open-Tile-List found we find a new start again, see "Find first/next start":

	Get next tile forward:

	- @TODO: Write down the next steps

	*/

	namespace internals {

		template <typename T, size_t N>
		inline size_t ArrayCount(T(&arr)[N]) {
			size_t result = sizeof(arr) / sizeof(arr[0]);
			return(result);
		}

		template <typename T, typename U, size_t N>
		inline void ArrayRemoveAndKeepOrder(T(&arr)[N], const U indexToRemove, U &count) {
			assert(indexToRemove < N);
			U oldListCount = count;
			assert(oldListCount > 0);
			for (U itemIndex = indexToRemove; itemIndex < oldListCount - 1; ++itemIndex) {
				T tmp = arr[itemIndex];
				arr[itemIndex] = arr[itemIndex + 1];
				arr[itemIndex + 1] = tmp;
			}
			count = oldListCount - 1;
		}

		inline Vec2i V2i(const int32_t x, const int32_t y) {
			Vec2i result = { x, y };
			return(result);
		}

		// Up, Right, Down, Left
		static const Vec2i TILETRACE_DIRECTIONS[] = { V2i(0, -1), V2i(1, 0), V2i(0, 1), V2i(-1, 0) };
		static const uint32_t TILETRACE_DIRECTION_COUNT = 4;

		inline bool IsEqual(const Vec2i &a, const Vec2i &b) {
			bool result = (a.x == b.x) && (a.y == b.y);
			return(result);
		}

		inline Vec2i Subtract(const Vec2i &a, const Vec2i &b) {
			Vec2i result = { a.x - b.x, a.y - b.y };
			return(result);
		}

		inline int32_t Dot(const Vec2i &a, const Vec2i &b) {
			int32_t result = a.x * b.x + a.y * b.y;
			return(result);
		}

		inline Tile MakeTile(int32_t x, int32_t y, int32_t isSolid) {
			Tile tile = {};
			tile.x = x;
			tile.y = y;
			tile.isSolid = isSolid;
			return(tile);
		}

		inline Edge MakeTileEdge(int32_t index, int32_t vertIndex0, int32_t vertIndex1, const Vec2i &tilePosition) {
			Edge result = {};
			result.index = index;
			result.vertIndex0 = vertIndex0;
			result.vertIndex1 = vertIndex1;
			result.tilePosition = tilePosition;
			return(result);
		}

		inline uint64_t ComputeTileHash(uint32_t x, uint32_t y) {
			uint64_t result = ((uint64_t)x << 32) | (uint64_t)y;
			return(result);
		}

		inline uint32_t ComputeTileIndex(const Vec2u &dimension, uint32_t x, uint32_t y) {
			uint32_t result = y * dimension.w + x;
			return(result);
		}

		inline int32_t IsTileSolid(std::vector<Tile> &tiles, const Vec2u &dimension, int32_t x, int32_t y) {
			int32_t result = false;
			if ((x >= 0 && x < (int32_t)dimension.w) && (y >= 0 && y < (int32_t)dimension.h)) {
				uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
				assert(tileIndex < dimension.w * dimension.h);
				result = tiles[tileIndex].isSolid > 0;
			}
			return(result);
		}

		inline Tile *GetTile(std::vector<Tile> &tiles, const Vec2u &dimension, uint32_t x, uint32_t y) {
			assert((x < (uint32_t)dimension.w) && (y < (uint32_t)dimension.h));
			uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
			return &tiles[tileIndex];
		}

		inline void SetTileSolid(std::vector<Tile> &tiles, const Vec2u &dimension, uint32_t x, uint32_t y, int32_t value) {
			assert((x < (uint32_t)dimension.w) && (y < (uint32_t)dimension.h));
			uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
			tiles[tileIndex].isSolid = value;
		}
		inline void RemoveTile(std::vector<Tile> &tiles, const Vec2u &dimension, uint32_t x, uint32_t y) {
			SetTileSolid(tiles, dimension, x, y, -1);
		}

		static Tile *GetFirstSolidTile(std::vector<Tile> &tiles, const Vec2u &dimension) {
			for (uint32_t tileY = 0; tileY < dimension.h; ++tileY) {
				for (uint32_t tileX = 0; tileX < dimension.w; ++tileX) {
					if (IsTileSolid(tiles, dimension, tileX, tileY)) {
						Tile *tile = GetTile(tiles, dimension, tileX, tileY);
						return tile;
					}
				}
			}
			return ftt_null;
		}

		static TileVertices CreateTileVertices(Tile *tile) {
			TileVertices result = {};
			result.verts[0] = V2i(tile->x, tile->y + 1);
			result.verts[1] = V2i(tile->x, tile->y);
			result.verts[2] = V2i(tile->x + 1, tile->y);
			result.verts[3] = V2i(tile->x + 1, tile->y + 1);
			return(result);
		}

		static TileIndices PushTileVertices(TileTracerData *traceState, Tile *tile) {
			TileIndices result = {};
			TileVertices tileVerts = CreateTileVertices(tile);
			assert(ArrayCount(tileVerts.verts) == ArrayCount(result.indices));
			for (uint32_t vertIndex = 0; vertIndex < ArrayCount(tileVerts.verts); ++vertIndex) {
				Vec2i vertex = tileVerts.verts[vertIndex];
				int32_t matchedMainVertexIndex = -1;
				for (uint32_t mainVertexIndex = 0; mainVertexIndex < traceState->mainVertices.size(); ++mainVertexIndex) {
					if (IsEqual(traceState->mainVertices[mainVertexIndex], vertex)) {
						matchedMainVertexIndex = mainVertexIndex;
						break;
					}
				}
				if (matchedMainVertexIndex == -1) {
					result.indices[vertIndex] = (int32_t)traceState->mainVertices.size();
					traceState->mainVertices.push_back(vertex);
				} else {
					result.indices[vertIndex] = matchedMainVertexIndex;
				}
			}
			return(result);
		}

		static TileEdges CreateTileEdges(TileIndices tileIndices, const Vec2u &tileCount, Tile *tile) {
			TileEdges result = {};
			uint32_t indexCount = (uint32_t)ArrayCount(tileIndices.indices);
			for (uint32_t index = 0; index < indexCount; ++index) {
				int32_t e0 = tileIndices.indices[index];
				int32_t e1 = tileIndices.indices[(index + 1) % indexCount];
				result.edges[result.count++] = MakeTileEdge(index, e0, e1, V2i(tile->x, tile->y));
			}
			return(result);
		}

		static TileEdges RemoveOverlapEdges(TileTracerData *traceState, TileEdges inputEdges) {
			TileEdges result = {};
			for (uint32_t edgeIndex = 0; edgeIndex < inputEdges.count; ++edgeIndex) {
				Edge inputEdge = inputEdges.edges[edgeIndex];
				bool addIt = true;
				for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < traceState->mainEdges.size(); ++mainEdgeIndex) {
					Edge mainEdge = traceState->mainEdges[mainEdgeIndex];
					if (inputEdge.vertIndex0 == mainEdge.vertIndex1 && inputEdge.vertIndex1 == mainEdge.vertIndex0) {
						addIt = false;
						traceState->mainEdges.erase(traceState->mainEdges.begin() + mainEdgeIndex);
						break;
					}
				}
				if (addIt) {
					result.edges[result.count++] = inputEdge;
				}
			}
			return(result);
		}

		static bool IsTileSharesCommonEdges(TileTracerData *traceState, TileVertices tileVertices) {
			uint32_t vertexCount = (uint32_t)ArrayCount(tileVertices.verts);
			for (uint32_t vertIndex = 0; vertIndex < vertexCount; ++vertIndex) {
				Vec2i tv0 = tileVertices.verts[vertIndex];
				Vec2i tv1 = tileVertices.verts[(vertIndex + 1) % vertexCount];
				for (uint32_t edgeIndex = 0; edgeIndex < traceState->mainEdges.size(); ++edgeIndex) {
					Edge edge = traceState->mainEdges[edgeIndex];
					Vec2i mv0 = traceState->mainVertices[edge.vertIndex0];
					Vec2i mv1 = traceState->mainVertices[edge.vertIndex1];
					if (IsEqual(tv0, mv1) && IsEqual(tv1, mv0)) {
						return true;
					}
				}
			}
			return false;
		}

		inline void RemoveSegmentVertex(ChainSegment *chainSegment, uint32_t index) {
			assert(index < chainSegment->vertices.size());
			chainSegment->vertices.erase(chainSegment->vertices.begin() + index);
		}

		static void ClearLineSegmentPoints(ChainSegment *segment, uint32_t firstIndex, uint32_t middleIndex, uint32_t lastIndex) {
			Vec2i last = segment->vertices[lastIndex];
			Vec2i middle = segment->vertices[middleIndex];
			Vec2i first = segment->vertices[firstIndex];
			Vec2i d1 = Subtract(last, middle);
			Vec2i d2 = Subtract(middle, first);
			int32_t d = Dot(d1, d2);
			if (d > 0) {
				RemoveSegmentVertex(segment, middleIndex);
			}
		}

		static void OptimizeChainSegment(ChainSegment *segment) {
			if (segment->vertices.size() > 2) {
				uint32_t lastIndex = (uint32_t)(segment->vertices.size() - 1);
				ClearLineSegmentPoints(segment, lastIndex - 2, lastIndex - 1, lastIndex);
			}
		}

		static void FinalizeChainSegment(ChainSegment *chainSegment) {
			if (chainSegment->vertices.size() > 2) {
				ClearLineSegmentPoints(chainSegment, (uint32_t)(chainSegment->vertices.size() - 1), 0, 1);
			}
			if (chainSegment->vertices.size() > 2) {
				ClearLineSegmentPoints(chainSegment, 0, (uint32_t)(chainSegment->vertices.size() - 1), (uint32_t)(chainSegment->vertices.size() - 2));
			}
		}

		inline void AddChainSegmentVertex(ChainSegment *chainSegment, const Vec2i &vertex) {
			chainSegment->vertices.push_back(vertex);
		}

		static bool ProcessTraverseNextEdge(TileTracerData *traceState) {
			for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < traceState->mainEdges.size(); ++mainEdgeIndex) {
				Edge *curEdge = &traceState->mainEdges[mainEdgeIndex];
				if (!curEdge->isInvalid && (curEdge->vertIndex0 == traceState->lastEdge->vertIndex1)) {
					// If v0 from current edge equals starting edge - then we are finished
					if (curEdge->vertIndex1 == traceState->startEdge->vertIndex0) {
						// We are done with this line segment - Set cur step to find next starting edge
						traceState->lastEdge = ftt_null;
						traceState->curStep = Step::TraverseFindStartingEdge;
						// Optimize and finalize shape
						OptimizeChainSegment(traceState->curChainSegment);
						FinalizeChainSegment(traceState->curChainSegment);
						// Add list vertex to the end again, because we have a fully closed chain
						AddChainSegmentVertex(traceState->curChainSegment, traceState->curChainSegment->vertices[0]);
					} else {
						// Now our current edge is the last edge
						traceState->lastEdge = curEdge;
						// Add always the first edge vertex to the list
						AddChainSegmentVertex(traceState->curChainSegment, traceState->mainVertices[curEdge->vertIndex1]);
						// Optimize shape
						OptimizeChainSegment(traceState->curChainSegment);
					}
					curEdge->isInvalid = true;
					return true;
				}
			}

			// We will come here for a line segment which is not fully closed, may have holes or something
			if (traceState->curChainSegment->vertices.size() > 0) {
				// We are done with this line segment - Set cur step to find next starting edge
				traceState->lastEdge = ftt_null;
				traceState->curStep = Step::TraverseFindStartingEdge;
				// Optimize and finalize shape
				OptimizeChainSegment(traceState->curChainSegment);
				FinalizeChainSegment(traceState->curChainSegment);
				return true;
			}

			// We are totally done, there is no line segment or anything to do
			// @TODO: An assert would be appropiate here
			traceState->curStep = Step::Done;
			return false;
		}

		static bool ProcessTraverseFindStartingEdge(TileTracerData *traceState) {
			bool result = false;

			// Find next free starting edge - at the start this is always null
			traceState->startEdge = ftt_null;
			int32_t startEdgeIndex = -1;
			for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < traceState->mainEdges.size(); ++mainEdgeIndex) {
				Edge *mainEdge = &traceState->mainEdges[mainEdgeIndex];
				if (!mainEdge->isInvalid) {
					startEdgeIndex = mainEdgeIndex;
					traceState->startEdge = mainEdge;
					break;
				}
			}

			if (traceState->startEdge != ftt_null) {
				// Continue when we found a starting edge and create the actual line segment for it
				result = true;
				traceState->lastEdge = traceState->startEdge;
				traceState->mainEdges[startEdgeIndex].isInvalid = true;
				traceState->curStep = Step::TraverseNextEdge;
				traceState->chainSegments.push_back(ChainSegment());
				traceState->curChainSegment = &traceState->chainSegments[traceState->chainSegments.size() - 1];
				AddChainSegmentVertex(traceState->curChainSegment, traceState->mainVertices[traceState->startEdge->vertIndex0]);
				AddChainSegmentVertex(traceState->curChainSegment, traceState->mainVertices[traceState->startEdge->vertIndex1]);
			} else {
				// We are completely done
				traceState->curStep = Step::Done;
			}

			return(result);
		}

		static void GetNextOpenTile(TileTracerData *traceState) {
			if (traceState->openList.size() > 0) {
				traceState->curTile = traceState->openList[traceState->openList.size() - 1];
				traceState->curStep = Step::FindNextTile;
			} else {
				traceState->curStep = Step::FindStart;
			}
		}

		static void RotateForward(TileTracerData *traceState) {
			if (traceState->curTile->traceDirection < (TILETRACE_DIRECTION_COUNT - 1)) {
				++traceState->curTile->traceDirection;
				traceState->curStep = Step::FindNextTile;
			} else {
				traceState->openList.pop_back();
				traceState->curStep = Step::GetNextOpenTile;
				GetNextOpenTile(traceState);
			}
		}

		static void AddTile(TileTracerData *traceState, Tile *tile) {
			// Add the start tile to the open list and remove it from the map
			traceState->openList.push_back(tile);
			RemoveTile(traceState->tiles, traceState->tileCount, tile->x, tile->y);

			// Create tile vertices/indices and edges for the next tile
			TileIndices tileIndices = PushTileVertices(traceState, tile);
			TileEdges tileEdges = CreateTileEdges(tileIndices, traceState->tileCount, tile);

			// Remove edges that overlap from the main edge list and the edge list for NextTile
			tileEdges = RemoveOverlapEdges(traceState, tileEdges);

			// Push the remaining edges to the main edges list
			for (uint32_t tileEdgeIndex = 0; tileEdgeIndex < tileEdges.count; ++tileEdgeIndex) {
				traceState->mainEdges.push_back(tileEdges.edges[tileEdgeIndex]);
			}
		}
	};

	ftt_api void InitTileTracer(TileTracerData *tracer, const Vec2u &tileCount, uint8_t *mapTiles) {
		assert(tracer != ftt_null);
		assert(mapTiles != ftt_null);

		using namespace internals;

		tracer->tileCount = tileCount;
		tracer->tiles.resize(tileCount.x  * tileCount.y);
		for (uint32_t tileY = 0; tileY < tileCount.h; ++tileY) {
			for (uint32_t tileX = 0; tileX < tileCount.w; ++tileX) {
				uint32_t tileIndex = ComputeTileIndex(tileCount, tileX, tileY);
				int32_t isSolid = mapTiles[tileIndex];
				Tile *tile = &tracer->tiles[tileIndex];
				*tile = MakeTile(tileX, tileY, isSolid);
			}
		}

		tracer->curStep = Step::FindStart;
		tracer->openList.clear();
		tracer->startTile = ftt_null;
		tracer->mainVertices.clear();
		tracer->mainEdges.clear();
		tracer->chainSegments.clear();

		tracer->curTile = ftt_null;
		tracer->nextTile = ftt_null;
	}

	ftt_api bool NextTileTraceStep(TileTracerData *tracer) {
		assert(tracer != ftt_null);

		using namespace internals;

		bool result = true;
		switch (tracer->curStep) {
			case Step::Done:
			{
				result = false;
			}; break;
			case Step::TraverseNextEdge:
			{
				result = ProcessTraverseNextEdge(tracer);
			}; break;
			case Step::TraverseFindStartingEdge:
			{
				result = ProcessTraverseFindStartingEdge(tracer);
			}; break;
			case Step::FindStart:
			{
				tracer->openList.clear();
				tracer->curTile = ftt_null;
				tracer->startTile = GetFirstSolidTile(tracer->tiles, tracer->tileCount);
				if (tracer->startTile != ftt_null) {
					// Add the start tile to the open list and build vertices and edges from it
					AddTile(tracer, tracer->startTile);

					// Set next step to get open tile and process it immediatitly
					tracer->curStep = Step::GetNextOpenTile;
					GetNextOpenTile(tracer);
				} else {
					// No start found, exit if we have not found any edges at all
					if (tracer->mainEdges.size() == 0) {
						result = false;
					} else {
						// Clear all chain segments
						tracer->chainSegments.clear();
						tracer->curStep = Step::TraverseFindStartingEdge;
					}
				}
			}; break;
			case Step::GetNextOpenTile:
			{
				GetNextOpenTile(tracer);
			}; break;
			case Step::FindNextTile:
			{
				assert(tracer->curTile != ftt_null);

				// Tile in the "forward" direction of the current tile
				int32_t nx = tracer->curTile->x + TILETRACE_DIRECTIONS[tracer->curTile->traceDirection].x;
				int32_t ny = tracer->curTile->y + TILETRACE_DIRECTIONS[tracer->curTile->traceDirection].y;
				tracer->nextTile = IsTileSolid(tracer->tiles, tracer->tileCount, nx, ny) ? GetTile(tracer->tiles, tracer->tileCount, nx, ny) : ftt_null;

				if (tracer->nextTile != ftt_null) {
					// Create tile vertices for next tile and check if it shares common edges
					TileVertices tileVertices = CreateTileVertices(tracer->nextTile);
					bool sharesCommonEdge = IsTileSharesCommonEdges(tracer, tileVertices);
					if (sharesCommonEdge) {
						// Add the next tile to the open list and build vertices and edges from it
						AddTile(tracer, tracer->nextTile);

						// Set next step to get open tile and process it immediatitly
						tracer->curStep = Step::GetNextOpenTile;
						GetNextOpenTile(tracer);
					} else {
						tracer->curStep = Step::RotateForward;
						RotateForward(tracer);
					}
				} else {
					tracer->curStep = Step::RotateForward;
					RotateForward(tracer);
				}
			}; break;
			case Step::RotateForward:
			{
				RotateForward(tracer);
			}; break;
			default:
				assert(!"Invalid default case!");
		}
		return(result);
	}

	ftt_api void RunTileTracer(TileTracerData *tracer) {
		assert(tracer != ftt_null);
		while (NextTileTraceStep(tracer)) {
		}
	}

	TileTracer::TileTracer(const Vec2u &tileCount, uint8_t *mapTiles) {
		data = {};
		InitTileTracer(&data, tileCount, mapTiles);
	}

	bool TileTracer::Next() {
		bool result = NextTileTraceStep(&data);
		return(result);
	}

	void TileTracer::Run() {
		RunTileTracer(&data);
	}

}
#endif