/*
-------------------------------------------------------------------------------
Name:
	Tile Trace Test

Description:
	Unit test suite for final_tiletrace.h. Exercises the contour tracer on solid
	tiles and on all four slope tile types (fttTileType), plus the public
	fttGetTileShapeCorners helper.

	Contains regression tests for slope support:
	- Every single slope traces to its expected triangle.
	- A slope leg cancels against an adjacent block edge (connectivity).
	- A block/slope corner is preserved by the chain optimizer (the collinearity
	  fix - a plain dot-product test used to collapse shallow slope corners).
	- A hill of slopes on a floor traces to one closed trapezoid.
	- Four slopes tile a diamond that traces to a single rotated square.

	Returns 0 when all tests pass, 1 otherwise.

Requirements:
	- C17 Compiler
	- final_tiletrace.h (no other dependencies)

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FTT_IMPLEMENTATION
#include <final_tiletrace.h>

#include <stdio.h>

static int g_total = 0;
static int g_failed = 0;
static const char *g_section = "";

static void Section(const char *name) {
	g_section = name;
	printf("\n[%s]\n", name);
}

static void CheckImpl(const bool ok, const char *expr, const int line) {
	++g_total;
	if (ok) {
		printf("  ok   %s\n", expr);
	} else {
		++g_failed;
		printf("  FAIL %s  (%s:%d)\n", expr, g_section, line);
	}
}

#define CHECK(cond) CheckImpl((cond), #cond, __LINE__)

// A closed contour expressed as a polygon (no duplicated closing vertex).
typedef struct ExpectedPolygon {
	fttVec2i verts[16];
	uint32_t count;
} ExpectedPolygon;

static bool VecEqual(fttVec2i a, fttVec2i b) {
	return a.x == b.x && a.y == b.y;
}

// Returns the produced segment's vertex count without the duplicated closing vertex.
static uint32_t SegmentUniqueVertexCount(const fttChainSegment *segment) {
	uint32_t count = (uint32_t)fttGetChainSegmentVertexCount(segment);
	if (count >= 2) {
		fttVec2i first = fttGetChainSegmentVertex(segment, 0);
		fttVec2i last = fttGetChainSegmentVertex(segment, count - 1);
		if (VecEqual(first, last)) {
			--count;
		}
	}
	return count;
}

// Matches a produced closed contour against an expected polygon, invariant to which
// vertex the traversal happened to start on (both are wound clockwise, so no reversal).
static bool SegmentMatchesPolygon(const fttChainSegment *segment, const ExpectedPolygon *expected) {
	uint32_t uniqueCount = SegmentUniqueVertexCount(segment);
	if (uniqueCount != expected->count) {
		return false;
	}
	if (uniqueCount == 0) {
		return true;
	}
	for (uint32_t startOffset = 0; startOffset < uniqueCount; ++startOffset) {
		bool allMatch = true;
		for (uint32_t i = 0; i < uniqueCount; ++i) {
			fttVec2i produced = fttGetChainSegmentVertex(segment, (startOffset + i) % uniqueCount);
			if (!VecEqual(produced, expected->verts[i])) {
				allMatch = false;
				break;
			}
		}
		if (allMatch) {
			return true;
		}
	}
	return false;
}

// Runs the tracer on a map and asserts it produces exactly one contour matching the expected polygon.
static bool TraceSingleContour(uint32_t width, uint32_t height, const uint8_t *map, const ExpectedPolygon *expected) {
	fttVec2u tileCount;
	tileCount.w = width;
	tileCount.h = height;

	fttTileTracer tracer;
	fttInitTileTracer(&tracer, tileCount, map, ftt_null);
	fttRunTileTracer(&tracer);

	bool result = false;
	if (fttGetChainSegmentCount(&tracer) == 1) {
		const fttChainSegment *segment = fttGetChainSegment(&tracer, 0);
		result = SegmentMatchesPolygon(segment, expected);
	}

	fttFreeTileTracer(&tracer);
	return result;
}

static uint32_t TraceSegmentCount(uint32_t width, uint32_t height, const uint8_t *map) {
	fttVec2u tileCount;
	tileCount.w = width;
	tileCount.h = height;

	fttTileTracer tracer;
	fttInitTileTracer(&tracer, tileCount, map, ftt_null);
	fttRunTileTracer(&tracer);

	uint32_t result = (uint32_t)fttGetChainSegmentCount(&tracer);
	fttFreeTileTracer(&tracer);
	return result;
}

static ExpectedPolygon MakePolygon3(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	ExpectedPolygon result;
	result.count = 3;
	result.verts[0] = (fttVec2i){ { x0, y0 } };
	result.verts[1] = (fttVec2i){ { x1, y1 } };
	result.verts[2] = (fttVec2i){ { x2, y2 } };
	return result;
}

static void TestShapeCorners(void) {
	Section("fttGetTileShapeCorners");

	fttVec2i corners[4];

	// An empty tile has no shape
	CHECK(fttGetTileShapeCorners(FTT_TILE_EMPTY, 3, 4, corners) == 0);

	// A solid tile has all four corners in clockwise order (bottom-left, top-left, top-right, bottom-right)
	uint32_t solidCount = fttGetTileShapeCorners(FTT_TILE_SOLID, 3, 4, corners);
	CHECK(solidCount == 4);
	CHECK(VecEqual(corners[0], (fttVec2i){ { 3, 5 } }));
	CHECK(VecEqual(corners[1], (fttVec2i){ { 3, 4 } }));
	CHECK(VecEqual(corners[2], (fttVec2i){ { 4, 4 } }));
	CHECK(VecEqual(corners[3], (fttVec2i){ { 4, 5 } }));

	// Each slope drops the corner opposite its right angle and keeps three corners
	uint32_t bottomRightCount = fttGetTileShapeCorners(FTT_TILE_SLOPE_BOTTOM_RIGHT, 0, 0, corners);
	CHECK(bottomRightCount == 3);
	CHECK(VecEqual(corners[0], (fttVec2i){ { 0, 1 } }) && VecEqual(corners[1], (fttVec2i){ { 1, 0 } }) && VecEqual(corners[2], (fttVec2i){ { 1, 1 } }));

	uint32_t bottomLeftCount = fttGetTileShapeCorners(FTT_TILE_SLOPE_BOTTOM_LEFT, 0, 0, corners);
	CHECK(bottomLeftCount == 3);
	CHECK(VecEqual(corners[0], (fttVec2i){ { 0, 1 } }) && VecEqual(corners[1], (fttVec2i){ { 0, 0 } }) && VecEqual(corners[2], (fttVec2i){ { 1, 1 } }));

	uint32_t topLeftCount = fttGetTileShapeCorners(FTT_TILE_SLOPE_TOP_LEFT, 0, 0, corners);
	CHECK(topLeftCount == 3);
	CHECK(VecEqual(corners[0], (fttVec2i){ { 0, 1 } }) && VecEqual(corners[1], (fttVec2i){ { 0, 0 } }) && VecEqual(corners[2], (fttVec2i){ { 1, 0 } }));

	uint32_t topRightCount = fttGetTileShapeCorners(FTT_TILE_SLOPE_TOP_RIGHT, 0, 0, corners);
	CHECK(topRightCount == 3);
	CHECK(VecEqual(corners[0], (fttVec2i){ { 0, 0 } }) && VecEqual(corners[1], (fttVec2i){ { 1, 0 } }) && VecEqual(corners[2], (fttVec2i){ { 1, 1 } }));
}

static void TestEmptyAndSolid(void) {
	Section("Empty and solid");

	// An empty map traces to nothing
	uint8_t empty[9] = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	};
	CHECK(TraceSegmentCount(3, 3, empty) == 0);

	// A single solid tile traces to its unit square
	uint8_t solid[9] = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	};
	ExpectedPolygon square;
	square.count = 4;
	square.verts[0] = (fttVec2i){ { 1, 1 } };
	square.verts[1] = (fttVec2i){ { 2, 1 } };
	square.verts[2] = (fttVec2i){ { 2, 2 } };
	square.verts[3] = (fttVec2i){ { 1, 2 } };
	CHECK(TraceSingleContour(3, 3, solid, &square));
}

static void TestSingleSlopes(void) {
	Section("Single slopes");

	uint8_t bottomRight[9] = {
		0, 0, 0,
		0, FTT_TILE_SLOPE_BOTTOM_RIGHT, 0,
		0, 0, 0,
	};
	ExpectedPolygon bottomRightTri = MakePolygon3(1, 2, 2, 1, 2, 2);
	CHECK(TraceSingleContour(3, 3, bottomRight, &bottomRightTri));

	uint8_t bottomLeft[9] = {
		0, 0, 0,
		0, FTT_TILE_SLOPE_BOTTOM_LEFT, 0,
		0, 0, 0,
	};
	ExpectedPolygon bottomLeftTri = MakePolygon3(1, 2, 1, 1, 2, 2);
	CHECK(TraceSingleContour(3, 3, bottomLeft, &bottomLeftTri));

	uint8_t topLeft[9] = {
		0, 0, 0,
		0, FTT_TILE_SLOPE_TOP_LEFT, 0,
		0, 0, 0,
	};
	ExpectedPolygon topLeftTri = MakePolygon3(1, 2, 1, 1, 2, 1);
	CHECK(TraceSingleContour(3, 3, topLeft, &topLeftTri));

	uint8_t topRight[9] = {
		0, 0, 0,
		0, FTT_TILE_SLOPE_TOP_RIGHT, 0,
		0, 0, 0,
	};
	ExpectedPolygon topRightTri = MakePolygon3(1, 1, 2, 1, 2, 2);
	CHECK(TraceSingleContour(3, 3, topRight, &topRightTri));
}

static void TestSlopeConnectivityAndCorner(void) {
	Section("Slope connectivity and corner preservation");

	// Two solids capped on the right by a bottom-left slope. The slope's left leg cancels
	// against the block edge (they merge into one region), and the top corner where the
	// block roof meets the hypotenuse must survive the optimizer.
	uint8_t cappedRow[12] = {
		0, 0, 0, 0,
		1, 1, FTT_TILE_SLOPE_BOTTOM_LEFT, 0,
		0, 0, 0, 0,
	};
	ExpectedPolygon capped;
	capped.count = 4;
	capped.verts[0] = (fttVec2i){ { 0, 2 } };
	capped.verts[1] = (fttVec2i){ { 0, 1 } };
	capped.verts[2] = (fttVec2i){ { 2, 1 } };
	capped.verts[3] = (fttVec2i){ { 3, 2 } };
	CHECK(TraceSingleContour(4, 3, cappedRow, &capped));
}

static void TestHillOnFloor(void) {
	Section("Hill of slopes on a floor");

	// A solid floor with a bottom-right slope ascending on the left and a bottom-left slope
	// descending on the right. The collinear hypotenuse points merge into single diagonal edges,
	// so the whole thing traces to one closed trapezoid.
	uint8_t hill[18] = {
		0, 0, 0, 0, 0, 0,
		FTT_TILE_SLOPE_BOTTOM_RIGHT, 1, 1, 1, 1, FTT_TILE_SLOPE_BOTTOM_LEFT,
		1, 1, 1, 1, 1, 1,
	};
	ExpectedPolygon trapezoid;
	trapezoid.count = 6;
	trapezoid.verts[0] = (fttVec2i){ { 0, 2 } };
	trapezoid.verts[1] = (fttVec2i){ { 1, 1 } };
	trapezoid.verts[2] = (fttVec2i){ { 5, 1 } };
	trapezoid.verts[3] = (fttVec2i){ { 6, 2 } };
	trapezoid.verts[4] = (fttVec2i){ { 6, 3 } };
	trapezoid.verts[5] = (fttVec2i){ { 0, 3 } };
	CHECK(TraceSingleContour(6, 3, hill, &trapezoid));
}

static void TestDiamond(void) {
	Section("Diamond of four slopes");

	// Four slopes tiling a 2x2 block form a diamond (rotated unit square around the block center).
	uint8_t diamond[16] = {
		0, 0, 0, 0,
		0, FTT_TILE_SLOPE_BOTTOM_RIGHT, FTT_TILE_SLOPE_BOTTOM_LEFT, 0,
		0, FTT_TILE_SLOPE_TOP_RIGHT, FTT_TILE_SLOPE_TOP_LEFT, 0,
		0, 0, 0, 0,
	};
	ExpectedPolygon rhombus;
	rhombus.count = 4;
	rhombus.verts[0] = (fttVec2i){ { 1, 2 } };
	rhombus.verts[1] = (fttVec2i){ { 2, 1 } };
	rhombus.verts[2] = (fttVec2i){ { 3, 2 } };
	rhombus.verts[3] = (fttVec2i){ { 2, 3 } };
	CHECK(TraceSingleContour(4, 4, diamond, &rhombus));
}

static void TestBlockMapRegression(void) {
	Section("Block map regression");

	// A hollow ring of solid tiles traces to two contours: an outer boundary and an inner hole.
	uint8_t ring[25] = {
		1, 1, 1, 1, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 1, 1, 1, 1,
	};
	CHECK(TraceSegmentCount(5, 5, ring) == 2);

	// A filled block of solids traces to a single rectangle
	uint8_t block[12] = {
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
	};
	ExpectedPolygon rect;
	rect.count = 4;
	rect.verts[0] = (fttVec2i){ { 0, 0 } };
	rect.verts[1] = (fttVec2i){ { 4, 0 } };
	rect.verts[2] = (fttVec2i){ { 4, 3 } };
	rect.verts[3] = (fttVec2i){ { 0, 3 } };
	CHECK(TraceSingleContour(4, 3, block, &rect));
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	printf("final_tiletrace.h unit test suite\n");

	TestShapeCorners();
	TestEmptyAndSolid();
	TestSingleSlopes();
	TestSlopeConnectivityAndCorner();
	TestHillOnFloor();
	TestDiamond();
	TestBlockMapRegression();

	printf("\n===============================================\n");
	printf("Result: %d/%d passed, %d failed\n", g_total - g_failed, g_total, g_failed);
	printf("===============================================\n");
	return (g_failed == 0) ? 0 : 1;
}
