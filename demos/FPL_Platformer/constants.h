#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <final_platform_layer.h>

#include <final_math.h>

#define GameAspect (16.0f / 9.0f)
#define WorldWidth 640.0f
#define WorldHeight (WorldWidth / GameAspect)
#define WorldRadiusW (WorldWidth * 0.5f)
#define WorldRadiusH (WorldHeight * 0.5f)

#define TileWidth 32.0f
#define TileHeight 32.0f

#define TileSize V2fInit(TileWidth, TileHeight)
#define TileRadius V2fInit(TileWidth * 0.5f, TileHeight * 0.5f)
#define Gravity V2fInit(0, -10.0f)

#define AABBExpand 10.0f

#define MaxContactCount 128

#define MaxEntityCount 1024
#define StartEntityID 1

#define StartMapTileID (StartEntityID + MaxEntityCount)

#define PlayerMaxSpeed 100.0f
#define PlayerWalkSpeed 30.0f
#define PlayerAirSpeed 40.0f
#define PlayerJumpVelocity (200.0f * 1.2f)
#define PlayerGroundFriction 0.2f
#define PlayerAirFriction 0.2f

#endif // CONSTANTS_H