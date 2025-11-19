#ifndef WORLD_H
#define WORLD_H

#include <final_math.h>
#include <final_memory.h>
#include "final_game.h"

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
#define MaxEntityCount 2048UL

#define EntityIDStart 1UL
#define EntityIDRange (MaxEntityCount - 1UL)
#define EntityIDEnd (EntityIDRange - 1UL)

#define MapTileIDStart (EntityIDEnd + 1UL)
#define MapTileIDEnd (UINT32_MAX - 1UL)
#define MapTileIDRange ((MapTileIDEnd - MapTileIDStart) + 1UL)


//
// World types
//
typedef struct Camera2D {
	Vec2f offset[2];
	float scale[2];
	float worldToPixels;
	float pixelsToWorld;
} Camera2D;

typedef struct Entities {
	union {
		struct {
			Entity player;
			Entity others[MaxEntityCount - 1];
		};
		Entity list[MaxEntityCount];
	};
	size_t count;
} Entities;

typedef struct World {
	Entities entities;
	Physics physics;
	Map map;
	fmemMemoryBlock memory;
	Camera2D camera;
} World;

//
// Public Functions
//
extern bool WorldInit(fmemMemoryBlock *memory, World *world);
extern bool WorldClear(World *world);
extern bool WorldLoad(World *world, const MapDefinition *mapDefinition);
extern void WorldUpdate(World *world, const Input *input);

#endif // WORLD_H
