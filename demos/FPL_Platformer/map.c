#include "map.h"

#include <json/json.h>

extern bool MapInit(fmemMemoryBlock *memory, Map *map) {
	if (memory == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(map);
	map->tileSize = TileSize;
	map->tileRadius = TileRadius;
	if (!fmemPushBlock(memory, &map->temporaryMemory, MAP_MEMORY_TRANSIENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for map transient memory
	}
	if (!fmemPushBlock(memory, &map->persistentMemory, MAP_MEMORY_PERSISTENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for map persistent memory
	}
	return true;
}

extern void MapClear(Map *map) {
	if (!MapIsValid(map)) {
		return;
	}
	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;
	map->width = map->height = 0;
	map->origin = V2iInit(0, 0);
	map->maxBounds.min = map->maxBounds.max = V2fInit(0.0f, 0.0f);
	map->solidTiles = fpl_null;
}

fpl_internal bool MapTilePosIsObstacleLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool result = MapTileTypeIsObstacle(tile.type);
	return result;
}

fpl_internal bool MapTilePosIsMaskableLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool isSolid = tile.type == TileType_Solid;
	if (isSolid) {
		if (local.x == 0 || local.x == widthMinusOne || local.y == 0 || local.y == heightMinusOne) {
			return false; // Outside border is not maskable, treated as non-visible
		}
		return true;
	}
	return false;
}

// Gets the tile area mask from the specified map and local position
fpl_internal TileAreaMask MapGetTileAreaMaskLocal(const Map *map, const int widthMinusOne, const heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map) || !MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, local)) {
		return TileAreaMask_None;
	}

	TileAreaMask result = TileAreaMask_None;

	// NOTE(final): Map is defined as top-down in memory, so up is negative and down is positive
	Vec2i topLeft = V2iAdd(local, V2iInit(-1, -1));
	Vec2i topCenter = V2iAdd(local, V2iInit(0, -1));
	Vec2i topRight = V2iAdd(local, V2iInit(1, -1));
	Vec2i leftSide = V2iAdd(local, V2iInit(-1, 0));
	Vec2i rightSide = V2iAdd(local, V2iInit(1, 0));
	Vec2i bottomLeft = V2iAdd(local, V2iInit(-1, 1));
	Vec2i bottomCenter = V2iAdd(local, V2iInit(0, 1));
	Vec2i bottomRight = V2iAdd(local, V2iInit(1, 1));

	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, topLeft) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, topLeft))
		result |= TileAreaMask_TopLeft;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, topCenter) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, topCenter))
		result |= TileAreaMask_TopCenter;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, topRight) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, topRight))
		result |= TileAreaMask_TopRight;

	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, leftSide) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, leftSide))
		result |= TileAreaMask_LeftSide;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, local))
		result |= TileAreaMask_Centroid;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, rightSide) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, rightSide))
		result |= TileAreaMask_RightSide;

	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, bottomLeft) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, bottomLeft))
		result |= TileAreaMask_BottomLeft;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, bottomCenter) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, bottomCenter))
		result |= TileAreaMask_BottomCenter;
	if (!MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, bottomRight) && MapTilePosIsMaskableLocal(map, widthMinusOne, heightMinusOne, bottomRight))
		result |= TileAreaMask_BottomRight;

	return result;
}

fpl_internal void MapUpdateAllAreaMasks(Map *map) {
	if (!MapIsValid(map)) {
		return;
	}
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i local = V2iInit(x, y);
			tile->areaMask = MapGetTileAreaMaskLocal(map, widthMinusOne, heightMinusOne, local);
	}
	}
}

fpl_internal void MapAutoSetAllGhostTiles(Map *map) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return;
	}

	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;

	// Clean ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (tile->type != TileType_Ghost) {
				continue;
			}
			if (!MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y - 1)) || !MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y + 1))) {
				tile->type = TileType_None;
			}
		}
	}

	// Set ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (MapTileTypeIsObstacle(tile->type)) {
				continue;
			}
			if (MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y - 1)) && MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y + 1))) {
				tile->type = TileType_Ghost;
			}
		}
	}
}

fpl_internal void *_MapAllocateJSONMemoryInternal(void *userData, const size_t size) {
	if (userData == fpl_null || size == 0) {
		return fpl_null;
	}
	fmemMemoryBlock *transientBlock = (fmemMemoryBlock *)userData;
	void *result = fmemPush(transientBlock, size, fmemPushFlags_Clear);
	return result;
}

fpl_internal bool MapDefinitionLoadFromFile(fmemMemoryBlock *memoryBlock, const char *dataPath, const char *filename, const char *levelName, MapDefinition *outDefinition) {
	if (memoryBlock == fpl_null || fplGetStringLength(filename) == 0 || fplGetStringLength(levelName) == 0 || outDefinition == fpl_null) {
		return false; // Invalid arguments
	}

	size_t levelNameLen = fplGetStringLength(levelName);

	char filePath[FPL_MAX_PATH_LENGTH];
	if(fplGetStringLength(dataPath) > 0) {
		fplCopyString(dataPath, filePath, fplArrayCount(filePath));
		fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);
	} else {
		fplCopyString(filename, filePath, fplArrayCount(filePath));
	}

	json_value_t *root = fpl_null;
	void *fileData = fpl_null;
	size_t fileSize = 0;

	bool result = false;

	fplFileHandle file;
	if (!fplFileOpenBinary(filePath, &file)) {
		return false; // File not found or invalid
	}
	bool fileOk = false;
	fileSize = fplFileGetSizeFromHandle(&file);
	if (fileSize > 0) {
		fileData = fmemPush(memoryBlock, fileSize, fmemPushFlags_Clear);
		if (fileData != fpl_null) {
			size_t read = fplFileReadBlock(&file, fileSize, fileData, fileSize);
			fileOk = read == fileSize;
		}
	}
	fplFileClose(&file);

	if (!fileOk) {
		goto failed; // Failed to load the entire file content
	}

	// Parse entire json file into a DOM with a single memory block (json.h)
	root = json_parse_ex(fileData, fileSize, json_parse_flags_default, _MapAllocateJSONMemoryInternal, memoryBlock, fpl_null);
	if (root == fpl_null || root->type != json_type_object) {
		goto failed; // Failed to parse the json data into a DOM
	}

	//
	// ldtk file format (Level Designer ToolKit)
	// 
	// "__header__" object -> contains ldtk specific infos, such as version
	// 
	// "defs" object -> contains the definition of layers, entities, tilesets, enums, etc.
	// 
	//   "layers" -> array of *layer* object
	//     "__type" or "__type" -> type of the layer: IntGrid, Entities
	//     "identifier" -> name of the layer definition
	//     "uid" -> simple id of the layer definition
	//     "gridSize" -> size of a tile in pixels
	// 
	//     "intGridValues" -> array of int grid value objects (when IntGrid type)
	//       "value" -> a uint32_t that defines a value, such as 1, 2, etc.
	//       "identifier" -> name of the value
	// 
	// "levels" -> contains the actual level data with all infos and layers:
	//   "identifier" -> name of the level
	//   "iid" -> guid of the level
	//   "pxWid" -> width in pixels
	//   "pxHei" -> height in pixels
	//   "worldX" -> ???
	//   "worldY" -> ???
	//   "layerInstances" -> contains the actual layers for the level such as IntGrid, Entities, etc.
	//     "__type" -> Type of the *layer*: IntGrid, Entities (there are more, but those are relavent for use right now)
	//     "__cWid" -> Number of horizontal tiles
	//     "__cHei" -> Number of vertical tiles
	//     "__gridSize" -> Number of vertical tiles
	//     "iid" -> Guid of the layer (required for finding the layer!)
	//     "levelId" -> Level number (not required for now)
	//     "intGridCsv" -> Flat 1D array of IntGrid tiles, see

	json_array_element_t *curArrElement, *curArrElement2;

	// Root of our JSON dom
	json_object_t *rootObj = (json_object_t *)root->payload;

	// Get "levels" array
	json_array_t *levelsArr = JSONValueAsArray(JSONObjectFindValueByName(rootObj, "levels"));
	if (levelsArr == fpl_null || levelsArr->length == 0) {
		goto failed; // No levels array found or empty
	}

	// Get "defs" object
	json_object_t *defsObj = JSONValueAsObject(JSONObjectFindValueByName(rootObj, "defs"));
	if (defsObj != fpl_null) {
		json_array_t *layersArr = JSONValueAsArray(JSONObjectFindValueByName(defsObj, "layers"));
		if (layersArr != fpl_null && layersArr->length > 0) {
			curArrElement = layersArr->start;
			while (curArrElement != fpl_null) {
				if (curArrElement->value != fpl_null && curArrElement->value->type == json_type_object) {
					json_object_t *layerObj = (json_object_t *)curArrElement->value->payload;
					String layerIdentifier = JSONValueAsString(JSONObjectFindValueByName(layerObj, "identifier"));
					String layerType = JSONValueAsString(JSONObjectFindValueByName(layerObj, "__type"));
					int layerUid = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "uid"), 0);
					int layerGridSize = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "gridSize"), 0);
					json_array_t *intGridValuesArray = JSONValueAsArray(JSONObjectFindValueByName(layerObj, "intGridValues"));
					if (intGridValuesArray != fpl_null && intGridValuesArray->length > 0) {
						curArrElement2 = intGridValuesArray->start;
						while (curArrElement2 != fpl_null) {
							if (curArrElement2->value != fpl_null && curArrElement2->value->type == json_type_object) {
								json_object_t *valueObj = (json_object_t *)curArrElement2->value->payload;
								int value = JSONValueAsS32(JSONObjectFindValueByName(valueObj, "value"), 0);
								String valueIdentifier = JSONValueAsString(JSONObjectFindValueByName(valueObj, "identifier"));
							}
							curArrElement2 = curArrElement2->next;
						}
					}
				}
				curArrElement = curArrElement->next;
			}
		}
	}

	// Find level object by identifier
	json_object_t *levelObj = fpl_null;
	curArrElement = levelsArr->start;
	while (curArrElement != fpl_null) {
		if (curArrElement->value != fpl_null && curArrElement->value->type == json_type_object) {
			json_object_t *level = curArrElement->value->payload;
			String levelIdentifier = JSONValueAsString(JSONObjectFindValueByName(level, "identifier"));
			if (fplIsStringEqualLen(levelName, levelNameLen, levelIdentifier.str, levelIdentifier.len)) {
				levelObj = (json_object_t *)level;
				break;
			}
		}
		curArrElement = curArrElement->next;
	}

	if (levelObj == fpl_null) {
		goto failed; // No level object found
	}

	// Do not pass any string values along directly!
	IID16 levelIid = IID16ParseStringType(JSONValueAsString(JSONObjectFindValueByName(levelObj, "iid")));
	int levelPxWid = JSONValueAsS32(JSONObjectFindValueByName(levelObj, "pxWid"), 0);
	int levelPxHei = JSONValueAsS32(JSONObjectFindValueByName(levelObj, "pxHei"), 0);

	// Find layer instances array
	json_array_t *levelLayerInstancesArr = JSONValueAsArray(JSONObjectFindValueByName(levelObj, "layerInstances"));
	if (levelLayerInstancesArr == fpl_null || levelLayerInstancesArr->length == 0) {
		goto failed; // No layer found
	}

	// Loop through all layer instances
	curArrElement = levelLayerInstancesArr->start;
	while (curArrElement != fpl_null) {
		if (curArrElement->value != fpl_null && curArrElement->value->type == json_type_object) {
			json_object_t *layerObj = (json_object_t *)curArrElement->value->payload;
			String layerIdentifier = JSONValueAsString(JSONObjectFindValueByName(layerObj, "__identifier"));
			String layerType = JSONValueAsString(JSONObjectFindValueByName(layerObj, "__type"));
			int layerCWid = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "__cWid"), 0);
			int layerCHei = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "__cHei"), 0);
			int layerGridSize = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "__gridSize"), 0);
			IID16 layerIid = IID16ParseStringType(JSONValueAsString(JSONObjectFindValueByName(layerObj, "iid")));
			int layerDefUid = JSONValueAsS32(JSONObjectFindValueByName(layerObj, "layerDefUid"), 0);
			if (fplIsStringEqual("IntGrid", layerType.str)) {
				json_array_t *intGridCsvArray = JSONValueAsArray(JSONObjectFindValueByName(layerObj, "intGridCsv"));
			} else if (fplIsStringEqual("Entities", layerType.str)) {
				json_array_t *entityInstancesArray = JSONValueAsArray(JSONObjectFindValueByName(layerObj, "entityInstances"));
			}
		}
		curArrElement = curArrElement->next;
	}

	fplClearStruct(outDefinition);

	return true;

failed:
	return false;
}

extern bool MapAssign(Map *map, const MapDefinition *definition) {
	if (map == fpl_null || definition == fpl_null) {
		return false; // Invalid arguments
	}

	uint32_t totalTileCount = definition->width * definition->height;

	size_t requiredSize = totalTileCount * sizeof(Tile);
	if (requiredSize > map->persistentMemory.size) {
		return false; // Insufficient persistent memory
	}

	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;

	map->solidTiles = (Tile *)fmemPush(&map->persistentMemory, requiredSize, fmemPushFlags_Clear);
	if (map->solidTiles == fpl_null) {
		return false; // Insufficient persistent memory (allocation failed, even though the required size was validated beforehand)
	}

	Vec2f mapArea = V2fInit(definition->width * map->tileSize.w, definition->height * map->tileSize.h);
	Vec2f mapBottomLeft = V2fInit(map->origin.x * map->tileSize.w, map->origin.y * map->tileSize.h);
	AABB2f maxBounds = AABB2fInitFromBottomLeft(mapBottomLeft, mapArea);

	map->width = definition->width;
	map->height = definition->height;
	map->origin = V2iInit(0, 0);
	map->maxBounds = maxBounds;

	for (uint32_t tileIndex = 0; tileIndex < totalTileCount; ++tileIndex) {
		TileType type = definition->tiles[tileIndex];
		map->solidTiles[tileIndex].id = MapTileIDStart + tileIndex;
		map->solidTiles[tileIndex].type = type;
	}

	MapAutoSetAllGhostTiles(map);

	MapUpdateAllAreaMasks(map);

	return true;
}

extern Tile MapGetTile(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null) {
		return TileEmpty;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return TileEmpty;
	}
	Tile result = map->solidTiles[local.y * map->width + local.x];
	return result;
}

extern bool MapSetTileType(Map *map, const Vec2i tilePos, const TileType type) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);

	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return false;
	}

	Tile *curTile = &map->solidTiles[local.y * map->width + local.x];
	curTile->type = type;

	MapAutoSetAllGhostTiles(map);

	MapUpdateAllAreaMasks(map);

	return true;
}

extern bool MapFindPositionByTile(const Map *map, const TileType type, Vec2i *outTilePos) {
	if (!MapIsValid(map) || outTilePos == fpl_null) {
		return false; // Invalid arguments
	}
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile tile = map->solidTiles[y * map->width + x];
			if (tile.type == type)
			{
				Vec2i local = V2iInit(x, y);
				*outTilePos = MapLocalTilePosToPublicTilePos(map, local);
				return true;
			}
		}
	}
	return false;
}

extern bool MapResizeToTilePos(Map *map, const Vec2i tilePos) {
	if (!MapIsValid(map)) {
		return false; // Invalid arguments
	}

	int mapWidthMinusOne = map->width - 1;
	int mapHeightMinusOne = map->height - 1;

	Vec2i oldOrigin = map->origin;

	Vec2i newOrigin = map->origin;

	Vec2i append = V2iInit(0, 0);

	if (tilePos.x < oldOrigin.x) {
		int xcount = oldOrigin.x - tilePos.x;
		fplAssert(xcount > 0);
		append.w += xcount;
		newOrigin.x -= xcount;
	} else if ((tilePos.x - oldOrigin.x) > mapWidthMinusOne) {
		int xcount = (tilePos.x - oldOrigin.x) - mapWidthMinusOne;
		fplAssert(xcount > 0);
		append.w += xcount;
	}

	if (tilePos.y < oldOrigin.y) {
		int ycount = oldOrigin.y - tilePos.y;
		fplAssert(ycount > 0);
		append.h += ycount;
		newOrigin.y -= ycount;
	} else if ((tilePos.y - oldOrigin.y) > mapHeightMinusOne) {
		int ycount = (tilePos.y - oldOrigin.y) - mapHeightMinusOne;
		fplAssert(ycount > 0);
		append.h += ycount;
	}

	if (append.w == 0 && append.h == 0) {
		return false; // Nothnig to resize
	}

	fplAssert(append.w >= 0 || append.h >= 0);

	Vec2i oldMapSize = V2iInit(map->width, map->height);
	Vec2i newMapSize = V2iInit(map->width + append.w, map->height + append.h);

	map->temporaryMemory.used = 0;

	fmemMemoryBlock tempBlock;
	fmemBeginTemporary(&map->temporaryMemory, &tempBlock);

	size_t requiredOldSize = oldMapSize.w * oldMapSize.h * sizeof(Tile);
	fplAssert(requiredOldSize <= tempBlock.size);

	Tile *oldTiles = (Tile *)fmemPush(&tempBlock, requiredOldSize, fmemPushFlags_None);
	fplMemoryCopy(map->solidTiles, requiredOldSize, oldTiles);

	map->persistentMemory.used = 0;
	size_t requiredNewSize = newMapSize.w * newMapSize.h * sizeof(Tile);
	fplAssert(requiredNewSize <= map->persistentMemory.size);

	Vec2f mapArea = V2fInit(newMapSize.w * map->tileSize.w, newMapSize.h * map->tileSize.h);
	Vec2f mapBottomLeft = V2fInit(newOrigin.x * map->tileSize.w, newOrigin.y * map->tileSize.h);
	AABB2f maxBounds = AABB2fInitFromBottomLeft(mapBottomLeft, mapArea);

	// Create new tiles and copy old tiles over it
	map->width = newMapSize.w;
	map->height = newMapSize.h;
	map->origin = newOrigin;
	map->maxBounds = maxBounds;
	map->solidTiles = (Tile *)fmemPush(&map->persistentMemory, requiredNewSize, fmemPushFlags_Clear);

	Vec2i offsetFromOldToNew = V2iSub(oldOrigin, newOrigin);

	if (offsetFromOldToNew.y > 0) {
		offsetFromOldToNew.y = 0;
	} else if (append.h > 0) {
		offsetFromOldToNew.y = append.h;
	}

	for (int y = 0; y < oldMapSize.h; ++y) {
		for (int x = 0; x < oldMapSize.w; ++x) {
			Vec2i oldLocal = V2iInit(x, y);
			Vec2i newLocal = V2iAdd(oldLocal, offsetFromOldToNew);
			fplAssert(newLocal.x >= 0 && newLocal.x < newMapSize.w);
			fplAssert(newLocal.y >= 0 && newLocal.y < newMapSize.h);
			map->solidTiles[newLocal.y * newMapSize.w + newLocal.x] = oldTiles[oldLocal.y * oldMapSize.w + oldLocal.x];
		}
	}

	// Re-assign id's
	for (int y = 0; y < newMapSize.h; ++y) {
		for (int x = 0; x < newMapSize.w; ++x) {
			uint32_t index = y * newMapSize.w + x;
			map->solidTiles[index].id = MapTileIDStart + index;
		}
	}

	MapUpdateAllAreaMasks(map);

	fmemEndTemporary(&map->temporaryMemory);

	return true;
}