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

[ ] Block tile contour tracing
[ ] Creating optimized chain segments

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
		Vec2i vertices[64];
		uint32_t vertexCount;
	};

	struct TileTracer {
		Vec2i tileCount;
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

	ftt_api TileTracer CreateTileTracer(const Vec2i &tileCount, uint8_t *mapTiles);
	ftt_api bool NextTileTraceStep(TileTracer &tracer);
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

	inline Tile MakeTile(const int32_t x, const int32_t y, const int32_t isSolid) {
		Tile tile = {};
		tile.x = x;
		tile.y = y;
		tile.isSolid = isSolid;
		return(tile);
	}

	inline Edge MakeTileEdge(const int32_t index, const int32_t vertIndex0, const int32_t vertIndex1, const Vec2i &tilePosition) {
		Edge result = {};
		result.index = index;
		result.vertIndex0 = vertIndex0;
		result.vertIndex1 = vertIndex1;
		result.tilePosition = tilePosition;
		return(result);
	}

	inline uint64_t ComputeTileHash(const uint32_t x, const uint32_t y) {
		uint64_t result = ((uint64_t)x << 32) | (uint64_t)y;
		return(result);
	}

	inline uint32_t ComputeTileIndex(const Vec2i &dimension, const uint32_t x, const uint32_t y) {
		uint32_t result = y * dimension.w + x;
		return(result);
	}

	inline bool IsTileSolid(std::vector<Tile> &tiles, const Vec2i &dimension, const int32_t x, const int32_t y) {
		bool result = false;
		if ((x >= 0 && x < dimension.w) && (y >= 0 && y < dimension.h)) {
			uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
			assert(tileIndex < (uint32_t)dimension.w * (uint32_t)dimension.h);
			result = tiles[tileIndex].isSolid > 0;
		}
		return(result);
	}

	inline Tile *GetTile(std::vector<Tile> &tiles, const Vec2i &dimension, const uint32_t x, const uint32_t y) {
		assert((x < (uint32_t)dimension.w) && (y < (uint32_t)dimension.h));
		uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
		return &tiles[tileIndex];
	}

	inline void SetTileSolid(std::vector<Tile> &tiles, const Vec2i &dimension, const uint32_t x, const uint32_t y, const int32_t value) {
		assert((x < (uint32_t)dimension.w) && (y < (uint32_t)dimension.h));
		uint32_t tileIndex = ComputeTileIndex(dimension, x, y);
		tiles[tileIndex].isSolid = value;
	}
	inline void RemoveTile(std::vector<Tile> &tiles, const Vec2i &dimension, const uint32_t x, const uint32_t y) {
		SetTileSolid(tiles, dimension, x, y, -1);
	}

	inline void CopyTiles(const std::vector<Tile> &sourceTiles, Tile *destTiles, const Vec2i &dimension) {
		uint32_t tileCount = dimension.w * dimension.h;
		for (uint32_t index = 0; index < tileCount; ++index) {
			destTiles[index] = sourceTiles[index];
		}
	}

	inline Tile *GetFirstSolidTile(std::vector<Tile> &tiles, const Vec2i &dimension) {
		for (int32_t tileY = 0; tileY < dimension.h; ++tileY) {
			for (int32_t tileX = 0; tileX < dimension.w; ++tileX) {
				if (IsTileSolid(tiles, dimension, tileX, tileY)) {
					Tile *tile = GetTile(tiles, dimension, tileX, tileY);
					return tile;
				}
			}
		}
		return nullptr;
	}

	inline TileVertices CreateTileVertices(const int32_t tileX, const int32_t tileY) {
		TileVertices result = {};
		result.verts[0] = V2i(tileX, tileY + 1);
		result.verts[1] = V2i(tileX, tileY);
		result.verts[2] = V2i(tileX + 1, tileY);
		result.verts[3] = V2i(tileY + 1, tileY + 1);
		return(result);
	}

	inline TileEdges CreateTileEdges(const TileIndices &tileIndices, const Vec2i &tileCount, const int32_t tileX, const int32_t tileY) {
		TileEdges result = {};
		size_t indexCount = ArrayCount(tileIndices.indices);
		for (size_t index = 0; index < indexCount; ++index) {
			int32_t e0 = tileIndices.indices[index];
			int32_t e1 = tileIndices.indices[(index + 1) % indexCount];
			result.edges[result.count++] = MakeTileEdge((int32_t)index, e0, e1, V2i(tileX, tileY));
		}
		return(result);
	}

	static TileIndices PushTileVertices(TileTracer &tracer, const int32_t tileX, const int32_t tileY) {
		TileIndices result = {};
		TileVertices tileVerts = CreateTileVertices(tileX, tileY);
		assert(ArrayCount(tileVerts.verts) == ArrayCount(result.indices));
		for (uint32_t vertIndex = 0; vertIndex < ArrayCount(tileVerts.verts); ++vertIndex) {
			Vec2i vertex = tileVerts.verts[vertIndex];
			int32_t matchedMainVertexIndex = -1;
			for (uint32_t mainVertexIndex = 0; mainVertexIndex < tracer.mainVertices.size(); ++mainVertexIndex) {
				if (IsEqual(tracer.mainVertices[mainVertexIndex], vertex)) {
					matchedMainVertexIndex = mainVertexIndex;
					break;
				}
			}
			if (matchedMainVertexIndex == -1) {
				result.indices[vertIndex] = (int32_t)tracer.mainVertices.size();
				tracer.mainVertices.push_back(vertex);
			} else {
				result.indices[vertIndex] = matchedMainVertexIndex;
			}
		}
		return(result);
	}

	static TileEdges RemoveOverlapEdges(TileTracer &tracer, const TileEdges &inputEdges) {
		TileEdges result = {};
		for (uint32_t edgeIndex = 0; edgeIndex < inputEdges.count; ++edgeIndex) {
			const Edge &inputEdge = inputEdges.edges[edgeIndex];
			bool addIt = true;
			for (size_t mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
				const Edge &mainEdge = tracer.mainEdges[mainEdgeIndex];
				if (inputEdge.vertIndex0 == mainEdge.vertIndex1 && inputEdge.vertIndex1 == mainEdge.vertIndex0) {
					addIt = false;
					tracer.mainEdges.erase(tracer.mainEdges.begin() + mainEdgeIndex);
					break;
				}
			}
			if (addIt) {
				result.edges[result.count++] = inputEdge;
			}
		}
		return(result);
	}

	static bool IsTileSharedCommonEdges(TileTracer &tracer, TileVertices tileVertices) {
		size_t vertexCount = ArrayCount(tileVertices.verts);
		for (size_t vertIndex = 0; vertIndex < vertexCount; ++vertIndex) {
			const Vec2i &tv0 = tileVertices.verts[vertIndex];
			const Vec2i &tv1 = tileVertices.verts[(vertIndex + 1) % vertexCount];
			for (size_t edgeIndex = 0; edgeIndex < tracer.mainEdges.size(); ++edgeIndex) {
				const Edge &edge = tracer.mainEdges[edgeIndex];
				const Vec2i &mv0 = tracer.mainVertices[edge.vertIndex0];
				const Vec2i &mv1 = tracer.mainVertices[edge.vertIndex1];
				if (IsEqual(tv0, mv1) && IsEqual(tv1, mv0)) {
					return true;
				}
			}
		}
		return false;
	}

	inline void RemoveSegmentVertex(ChainSegment *chainSegment, uint32_t index) {
		ArrayRemoveAndKeepOrder<Vec2i, uint32_t>(chainSegment->vertices, index, chainSegment->vertexCount);
	}

	static void ClearLineSegmentPoints(ChainSegment *segment, uint32_t firstIndex, uint32_t middleIndex, uint32_t lastIndex) {
		const Vec2i &last = segment->vertices[lastIndex];
		const Vec2i &middle = segment->vertices[middleIndex];
		const Vec2i &first = segment->vertices[firstIndex];
		Vec2i d1 = Subtract(last, middle);
		Vec2i d2 = Subtract(middle, first);
		int32_t d = Dot(d1, d2);
		if (d > 0) {
			RemoveSegmentVertex(segment, middleIndex);
		}
	}

	static void OptimizeChainSegment(ChainSegment *segment) {
		if (segment->vertexCount > 2) {
			uint32_t lastIndex = segment->vertexCount - 1;
			ClearLineSegmentPoints(segment, lastIndex - 2, lastIndex - 1, lastIndex);
		}
	}

	static void FinalizeChainSegment(ChainSegment *chainSegment) {
		if (chainSegment->vertexCount > 2) {
			ClearLineSegmentPoints(chainSegment, chainSegment->vertexCount - 1, 0, 1);
		}
		if (chainSegment->vertexCount > 2) {
			ClearLineSegmentPoints(chainSegment, 0, chainSegment->vertexCount - 1, chainSegment->vertexCount - 2);
		}
	}

	inline void AddChainSegmentVertex(ChainSegment *chainSegment, const Vec2i &vertex) {
		assert(chainSegment->vertexCount < ArrayCount(chainSegment->vertices));
		chainSegment->vertices[chainSegment->vertexCount++] = vertex;
	}

	static bool ProcessTraverseNextEdge(TileTracer &tracer) {
		for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
			Edge *curEdge = &tracer.mainEdges[mainEdgeIndex];
			if (!curEdge->isInvalid && (curEdge->vertIndex0 == tracer.lastEdge->vertIndex1)) {
				// If v0 from current edge equals starting edge - then we are finished
				if (curEdge->vertIndex1 == tracer.startEdge->vertIndex0) {
					// We are done with this line segment - Set cur step to find next starting edge
					tracer.lastEdge = nullptr;
					tracer.curStep = Step::TraverseFindStartingEdge;
					// Optimize and finalize shape
					OptimizeChainSegment(tracer.curChainSegment);
					FinalizeChainSegment(tracer.curChainSegment);
					// Add list vertex to the end again, because we have a fully closed chain
					AddChainSegmentVertex(tracer.curChainSegment, tracer.curChainSegment->vertices[0]);
				} else {
					// Now our current edge is the last edge
					tracer.lastEdge = curEdge;
					// Add always the first edge vertex to the list
					AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[curEdge->vertIndex1]);
					// Optimize shape
					OptimizeChainSegment(tracer.curChainSegment);
				}
				curEdge->isInvalid = true;
				return true;
			}
		}

		// We will come here for a line segment which is not fully closed, may have holes or something
		if (tracer.curChainSegment->vertexCount > 0) {
			// We are done with this line segment - Set cur step to find next starting edge
			tracer.lastEdge = nullptr;
			tracer.curStep = Step::TraverseFindStartingEdge;
			// Optimize and finalize shape
			OptimizeChainSegment(tracer.curChainSegment);
			FinalizeChainSegment(tracer.curChainSegment);
			return true;
		}

		// We are totally done, there is no line segment or anything to do
		// @TODO: An assert would be appropiate here
		tracer.curStep = Step::Done;
		return false;
	}

	static bool ProcessTraverseFindStartingEdge(TileTracer &tracer) {
		bool result = false;

		// Find next free starting edge - at the start this is always null
		tracer.startEdge = nullptr;
		int32_t startEdgeIndex = -1;
		for (uint32_t mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
			Edge *mainEdge = &tracer.mainEdges[mainEdgeIndex];
			if (!mainEdge->isInvalid) {
				startEdgeIndex = mainEdgeIndex;
				tracer.startEdge = mainEdge;
				break;
			}
		}

		if (tracer.startEdge != nullptr) {
			// Continue when we found a starting edge and create the actual line segment for it
			result = true;
			tracer.lastEdge = tracer.startEdge;
			tracer.mainEdges[startEdgeIndex].isInvalid = true;
			tracer.curStep = Step::TraverseNextEdge;
			tracer.chainSegments.push_back({});
			tracer.curChainSegment = &tracer.chainSegments[tracer.chainSegments.size() - 1];
			AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[tracer.startEdge->vertIndex0]);
			AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[tracer.startEdge->vertIndex1]);
		} else {
			// We are completely done
			tracer.curStep = Step::Done;
		}

		return(result);
	}

	ftt_api TileTracer CreateTileTracer(const Vec2i &tileCount, uint8_t *mapTiles) {
		TileTracer result = TileTracer();
		result.tileCount = tileCount;
		result.tiles.reserve(tileCount.x  * tileCount.y);
		for (int32_t tileY = 0; tileY < tileCount.h; ++tileY) {
			for (int32_t tileX = 0; tileX < tileCount.w; ++tileX) {
				uint32_t tileIndex = ComputeTileIndex(tileCount, tileX, tileY);
				bool isSolid = mapTiles[tileIndex] > 0;
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

	static void GetNextOpenTile(TileTracer &tracer) {
		if (tracer.openList.size() > 0) {
			tracer.curTile = tracer.openList[tracer.openList.size() - 1];
			tracer.curStep = Step::FindNextTile;
		} else {
			tracer.curStep = Step::FindStart;
		}
	}

	static void RotateForward(TileTracer &tracer) {
		if (tracer.curTile->traceDirection < (TILETRACE_DIRECTION_COUNT - 1)) {
			++tracer.curTile->traceDirection;
			tracer.curStep = Step::FindNextTile;
		} else {
			tracer.openList.pop_back();
			tracer.curStep = Step::GetNextOpenTile;
			GetNextOpenTile(tracer);
		}
	}

	static void AddTile(TileTracer &tracer, Tile *tile) {
		// Add the start tile to the open list and remove it from the map
		tracer.openList.push_back(tile);
		RemoveTile(tracer.tiles, tracer.tileCount, tile->x, tile->y);

		// Create tile vertices/indices and edges for the next tile
		TileIndices tileIndices = PushTileVertices(tracer, tile->x, tile->y);
		TileEdges tileEdges = CreateTileEdges(tileIndices, tracer.tileCount, tile->x, tile->y);

		// Remove edges that overlap from the main edge list and the edge list for NextTile
		tileEdges = RemoveOverlapEdges(tracer, tileEdges);

		// Push the remaining edges to the main edges list
		for (uint32_t tileEdgeIndex = 0; tileEdgeIndex < tileEdges.count; ++tileEdgeIndex) {
			tracer.mainEdges.push_back(tileEdges.edges[tileEdgeIndex]);
		}
	}

	ftt_api bool NextTileTraceStep(TileTracer &tracer) {
		bool result = true;
		switch (tracer.curStep) {
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
				tracer.openList.clear();
				tracer.curTile = nullptr;
				tracer.startTile = GetFirstSolidTile(tracer.tiles, tracer.tileCount);
				if (tracer.startTile != nullptr) {
					// Add the start tile to the open list and build vertices and edges from it
					AddTile(tracer, tracer.startTile);

					// Set next step to get open tile and process it immediatitly
					tracer.curStep = Step::GetNextOpenTile;
					GetNextOpenTile(tracer);
				} else {
					// No start found, exit if we have not found any edges at all
					if (tracer.mainEdges.size() == 0) {
						result = false;
					} else {
						// Clear all chain segments
						tracer.chainSegments.clear();
						tracer.curStep = Step::TraverseFindStartingEdge;
					}
				}
			}; break;
			case Step::GetNextOpenTile:
			{
				GetNextOpenTile(tracer);
			}; break;
			case Step::FindNextTile:
			{
				assert(tracer.curTile != nullptr);

				// Tile in the "forward" direction of the current tile
				int32_t nx = tracer.curTile->x + TILETRACE_DIRECTIONS[tracer.curTile->traceDirection].x;
				int32_t ny = tracer.curTile->y + TILETRACE_DIRECTIONS[tracer.curTile->traceDirection].y;
				tracer.nextTile = IsTileSolid(tracer.tiles, tracer.tileCount, nx, ny) ? GetTile(tracer.tiles, tracer.tileCount, nx, ny) : nullptr;

				if (tracer.nextTile != nullptr) {
					// Create tile vertices for next tile and check if it shares common edges
					TileVertices tileVertices = CreateTileVertices(tracer.nextTile->x, tracer.nextTile->y);
					bool sharesCommonEdge = IsTileSharedCommonEdges(tracer, tileVertices);
					if (sharesCommonEdge) {
						// Add the next tile to the open list and build vertices and edges from it
						AddTile(tracer, tracer.nextTile);

						// Set next step to get open tile and process it immediatitly
						tracer.curStep = Step::GetNextOpenTile;
						GetNextOpenTile(tracer);
					} else {
						tracer.curStep = Step::RotateForward;
						RotateForward(tracer);
					}
				} else {
					tracer.curStep = Step::RotateForward;
					RotateForward(tracer);
				}
			}; break;
			case Step::RotateForward:
			{
				RotateForward(tracer);
			}; break;
			default:
				assert(!"Invalid code path!");
				break;
		}
		return(result);
	}
}
#endif