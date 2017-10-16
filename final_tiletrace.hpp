/**
* @file final_tiletrace.hpp
* @version v1.0
* @author Torsten Spaete
* @brief Final TileTrace (FTT) - a open source single file header c++ contour tile tracing library.
*
* This creates chainshapes out of any solid tilemap based on a contour tracing algorythmn.
*
* @mainpage
* Summary of the Final TileTrace (FTT) project.
* Please see @ref final_tiletrace.hpp for more details.
*/

/*
final_tiletrace.hpp
Open-Source Single-File Header C++ Library by Torsten Spaete

This creates chainshapes out of any solid tilemap based on a contour tracing algorythmn.
It will try to create as less chain shapes as possible.

- Supports block tiles only!

# Requirements

- STL (Standard Template Library)

# HOW TO USE

# EXAMPLES

# PREPROCESSOR OVERRIDES

# FEATURES

[X] Block tile contour tracing
[X] Creating optimized chain segments

# TODO

[ ] Finish algorythmn documentation

# LICENSE

MIT License

Copyright (c) 2017 Torsten Spaete
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

- v1.0 alpha:
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

namespace ftt {
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

	enum class Direction {
		Up,
		Right,
		Down,
		Left,
	};

	enum class Step {
		None,
		FindStart,
		GetNextOpenTile,
		FindNextTile,
		RotateForward,
		TraverseFindStartingEdge,
		TraverseNextEdge,
		Done,
	};

	struct Tile {
		int32_t x, y;
		uint32_t traceDirection;
		int32_t isSolid;
	};

	struct Edge {
		int32_t index;
		int32_t vertIndex0, vertIndex1;
		Vec2i tilePosition;
		bool isInvalid;
	};

	struct TileVertices {
		Vec2i verts[4];
	};
	struct TileIndices {
		int32_t indices[4];
	};
	struct TileEdges {
		Edge edges[4];
		uint32_t count;
	};

	struct ChainSegment {
		std::vector<Vec2i> vertices;
	};

	struct TileTracer {
		Vec2u tileCount;
		std::vector<Tile> tiles;
		Step curStep;
		Tile *startTile;
		Tile *curTile;
		Tile *nextTile;
		Edge *startEdge;
		Edge *lastEdge;
		ChainSegment *curChainSegment;
		std::vector<Tile *> openList;
		std::vector<Vec2i> mainVertices;
		std::vector<Edge> mainEdges;
		std::vector<ChainSegment> chainSegments;
	};

	ftt_api TileTracer CreateTileTracer(const Vec2u &tileCount, uint8_t *mapTiles);
	ftt_api bool NextTileTraceStep(TileTracer *tracer);
};
#endif

#if defined(FTT_IMPLEMENTATION) && !defined(FTT_IMPLEMENTED)
#	define FTT_IMPLEMENTED

#include <assert.h>

namespace ftt {
	/*
	https://en.wikipedia.org/wiki/Moore_neighborhood

	How this algotythm works:

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
	const static Vec2i TILETRACE_DIRECTIONS[] = { V2i(0, -1), V2i(1, 0), V2i(0, 1), V2i(-1, 0) };
	const static uint32_t TILETRACE_DIRECTION_COUNT = 4;

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
		return nullptr;
	}

	static TileVertices CreateTileVertices(Tile *tile) {
		TileVertices result = {};
		result.verts[0] = V2i(tile->x, tile->y + 1);
		result.verts[1] = V2i(tile->x, tile->y);
		result.verts[2] = V2i(tile->x + 1, tile->y);
		result.verts[3] = V2i(tile->x + 1, tile->y + 1);
		return(result);
	}

	static TileIndices PushTileVertices(TileTracer *traceState, Tile *tile) {
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

	static TileEdges RemoveOverlapEdges(TileTracer *traceState, TileEdges inputEdges) {
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

	static bool IsTileSharedCommonEdges(TileTracer *traceState, TileVertices tileVertices) {
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

	static bool ProcessTraverseNextEdge(TileTracer *traceState) {
		for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < traceState->mainEdges.size(); ++mainEdgeIndex) {
			Edge *curEdge = &traceState->mainEdges[mainEdgeIndex];
			if (!curEdge->isInvalid && (curEdge->vertIndex0 == traceState->lastEdge->vertIndex1)) {
				// If v0 from current edge equals starting edge - then we are finished
				if (curEdge->vertIndex1 == traceState->startEdge->vertIndex0) {
					// We are done with this line segment - Set cur step to find next starting edge
					traceState->lastEdge = nullptr;
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
			traceState->lastEdge = nullptr;
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

	static bool ProcessTraverseFindStartingEdge(TileTracer *traceState) {
		bool result = false;

		// Find next free starting edge - at the start this is always null
		traceState->startEdge = nullptr;
		int32_t startEdgeIndex = -1;
		for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < traceState->mainEdges.size(); ++mainEdgeIndex) {
			Edge *mainEdge = &traceState->mainEdges[mainEdgeIndex];
			if (!mainEdge->isInvalid) {
				startEdgeIndex = mainEdgeIndex;
				traceState->startEdge = mainEdge;
				break;
			}
		}

		if (traceState->startEdge != nullptr) {
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

	static void GetNextOpenTile(TileTracer *traceState) {
		if (traceState->openList.size() > 0) {
			traceState->curTile = traceState->openList[traceState->openList.size() - 1];
			traceState->curStep = Step::FindNextTile;
		} else {
			traceState->curStep = Step::FindStart;
		}
	}

	static void RotateForward(TileTracer *traceState) {
		if (traceState->curTile->traceDirection < (TILETRACE_DIRECTION_COUNT - 1)) {
			++traceState->curTile->traceDirection;
			traceState->curStep = Step::FindNextTile;
		} else {
			traceState->openList.pop_back();
			traceState->curStep = Step::GetNextOpenTile;
			GetNextOpenTile(traceState);
		}
	}

	static void AddTile(TileTracer *traceState, Tile *tile) {
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

	ftt_api TileTracer CreateTileTracer(const Vec2u &tileCount, uint8_t *mapTiles) {
		TileTracer result = TileTracer();
		result.tileCount = tileCount;
		result.tiles.resize(tileCount.x  * tileCount.y);
		for (uint32_t tileY = 0; tileY < tileCount.h; ++tileY) {
			for (uint32_t tileX = 0; tileX < tileCount.w; ++tileX) {
				uint32_t tileIndex = ComputeTileIndex(tileCount, tileX, tileY);
				int32_t isSolid = mapTiles[tileIndex];
				Tile *tile = &result.tiles[tileIndex];
				*tile = MakeTile(tileX, tileY, isSolid);
			}
		}

		result.curStep = Step::FindStart;
		result.openList.clear();
		result.startTile = nullptr;
		result.mainVertices.clear();
		result.mainEdges.clear();
		result.chainSegments.clear();

		result.curTile = nullptr;
		result.nextTile = nullptr;
		return(result);
	}

	ftt_api bool NextTileTraceStep(TileTracer *traceState) {
		bool result = true;
		switch (traceState->curStep) {
			case Step::Done:
			{
				result = false;
			}; break;
			case Step::TraverseNextEdge:
			{
				result = ProcessTraverseNextEdge(traceState);
			}; break;
			case Step::TraverseFindStartingEdge:
			{
				result = ProcessTraverseFindStartingEdge(traceState);
			}; break;
			case Step::FindStart:
			{
				traceState->openList.clear();
				traceState->curTile = nullptr;
				traceState->startTile = GetFirstSolidTile(traceState->tiles, traceState->tileCount);
				if (traceState->startTile != nullptr) {
					// Add the start tile to the open list and build vertices and edges from it
					AddTile(traceState, traceState->startTile);

					// Set next step to get open tile and process it immediatitly
					traceState->curStep = Step::GetNextOpenTile;
					GetNextOpenTile(traceState);
				} else {
					// No start found, exit if we have not found any edges at all
					if (traceState->mainEdges.size() == 0) {
						result = false;
					} else {
						// Clear all chain segments
						traceState->chainSegments.clear();
						traceState->curStep = Step::TraverseFindStartingEdge;
					}
				}
			}; break;
			case Step::GetNextOpenTile:
			{
				GetNextOpenTile(traceState);
			}; break;
			case Step::FindNextTile:
			{
				assert(traceState->curTile != nullptr);

				// Tile in the "forward" direction of the current tile
				int32_t nx = traceState->curTile->x + TILETRACE_DIRECTIONS[traceState->curTile->traceDirection].x;
				int32_t ny = traceState->curTile->y + TILETRACE_DIRECTIONS[traceState->curTile->traceDirection].y;
				traceState->nextTile = IsTileSolid(traceState->tiles, traceState->tileCount, nx, ny) ? GetTile(traceState->tiles, traceState->tileCount, nx, ny) : nullptr;

				if (traceState->nextTile != nullptr) {
					// Create tile vertices for next tile and check if it shares common edges
					TileVertices tileVertices = CreateTileVertices(traceState->nextTile);
					bool sharesCommonEdge = IsTileSharedCommonEdges(traceState, tileVertices);
					if (sharesCommonEdge) {
						// Add the next tile to the open list and build vertices and edges from it
						AddTile(traceState, traceState->nextTile);

						// Set next step to get open tile and process it immediatitly
						traceState->curStep = Step::GetNextOpenTile;
						GetNextOpenTile(traceState);
					} else {
						traceState->curStep = Step::RotateForward;
						RotateForward(traceState);
					}
				} else {
					traceState->curStep = Step::RotateForward;
					RotateForward(traceState);
				}
			}; break;
			case Step::RotateForward:
			{
				RotateForward(traceState);
			}; break;
			default:
				assert(!"Invalid default case!");
		}
		return(result);
	}
}
#endif