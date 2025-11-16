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
#define TileHalfExt V2fInit(TileWidth * 0.5f, TileHeight * 0.5f)
#define Gravity V2fInit(0, -10.0f)


#endif // CONSTANTS_H