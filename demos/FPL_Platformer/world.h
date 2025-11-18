#ifndef WORLD_H
#define WORLD_H

#include <final_math.h>

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

#endif // WORLD_H
