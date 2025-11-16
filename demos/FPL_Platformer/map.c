#include "map.h"

extern bool MapFindPositionByTile(const Map *map, const uint32_t type, Vec2i *outTilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null) {
		return false;
	}
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			uint32_t tile = MapGetTile(map, V2iInit(x, y));
			if (tile == type)
			{
				*outTilePos = V2iInit(x, y);
				return true;
			}
		}
	}
	return false;
}