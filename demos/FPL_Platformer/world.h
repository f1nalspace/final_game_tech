#ifndef WORLD_H
#define WORLD_H

#include <final_math.h>

#include <final_memory.h>

#include "entity.h"
#include "map.h"
#include "physics.h"

//
// Fixed world definition
//
#define WorldAspect (16.0f / 9.0f)
#define WorldWidth 640.0f
#define WorldHeight (WorldWidth / WorldAspect)
#define WorldRadiusW (WorldWidth * 0.5f)
#define WorldRadiusH (WorldHeight * 0.5f)

//
// Game / World states
//
#define MaxContactCount 128

#define MaxEntityCount 1024

#define StartEntityID 1
#define StartMapTileID (StartEntityID + MaxEntityCount)

//
// World types
//
typedef struct Camera2D {
	Vec2f offset[2];
	float scale[2];
	float worldToPixels;
	float pixelsToWorld;
} Camera2D;

typedef struct World {
	Map map;
	fmemMemoryBlock memory;
	Contact contacts[MaxContactCount];
	Entity player;
	Camera2D camera;
	uint32_t numContacts;
} World;

//
// Public Functions
//
extern bool WorldInit(fmemMemoryBlock *memory, World *world);
extern bool WorldLoadMap(World *world, const Map *map);

#endif // WORLD_H
