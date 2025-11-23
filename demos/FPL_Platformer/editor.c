#include "editor.h"

#include <final_game.h>

static bool EditorCanTileBePlaced(const Editor *editor, const Map *map, const Entity *player, const Vec2i tilePos) {
	AABB2f playerCurrentBounds = EntityGetCurrentBounds(player);
	Vec2f mouseWorldPos = MapTileCoordsToWorld(map, tilePos);
	AABB2f tileBounds = AABB2fInitFromBottomLeft(mouseWorldPos, TileSize);
	bool isOverlap = AABB2fIsOverlap(&tileBounds, &playerCurrentBounds);
	return !isOverlap;
}

static void EditorPaintTile(Editor *editor, Map *map, const Vec2i tilePos) {
	if (MapIsTileInside(map, tilePos)) {
		editor->drawTilePos = tilePos;
		MapSetTileType(map, tilePos, editor->drawTile);
		fplDebugFormatOut("Paint tile %d x %d with %d", tilePos.x, tilePos.y, editor->drawTile);
	} else {
		// NOTE(final): Resize returns a new tile position, due to offset change
		if (MapResizeToTilePos(map, tilePos)) {
			//MapSetTileType(map, tilePos, editor->drawTile);
			editor->drawTilePos = tilePos;
			editor->isDrawing = false;
		}
	}
}
fpl_extern bool EditorInit(fmemMemoryBlock *gameMemory, Editor *editor, GameAssets *assets, World *world) {
	if (gameMemory == fpl_null || editor == fpl_null || assets == fpl_null || world == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(editor);
	if (!fmemPushBlock(gameMemory, &editor->transientMemory, EDITOR_MEMORY_TRANSIENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for editor transient memory
	}
	if (!fmemPushBlock(gameMemory, &editor->persistentMemory, EDITOR_MEMORY_PERSISTENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for editor persistent memory
	}
	editor->world = world;
	editor->assets = assets;

	editor->cameraTranslation = V2fInit(0.0f, 0.0f);
	editor->cameraScale = 1.0f;

	return true;
}

fpl_extern void EditorInput(Editor *editor, const Input *input) {
	World *world = editor->world;
	Map *map = &world->map;
	Entity *player = &world->entities.player;
	Vec2i mouseTilePos = MapWorldCoordsToTile(map, editor->mouseWorldPos);

	if (ButtonIsDown(input->mouse.left)) {
		if (!editor->isDrawing) {
			if (EditorCanTileBePlaced(editor, map, player, mouseTilePos)) {
				editor->isDrawing = true;
				editor->drawTilePos = mouseTilePos;
				if (MapIsTileInside(map, mouseTilePos)) {
					Tile tile = MapGetTile(map, mouseTilePos);
					editor->drawTile = tile.type != TileType_Solid ? TileType_Solid : TileType_None;
				} else {
					editor->drawTile = TileType_Solid;
				}
				EditorPaintTile(editor, map, mouseTilePos);
			}
		} else {
			if (EditorCanTileBePlaced(editor, map, player, mouseTilePos)) {
				fplAssert(editor->drawTile != UINT32_MAX);
				if (!V2iEquals(mouseTilePos, editor->drawTilePos)) {
					EditorPaintTile(editor, map, mouseTilePos);
				}
			}
		}
	} else {
		editor->drawTile = UINT32_MAX;
		editor->drawTilePos = mouseTilePos;
		editor->isDrawing = false;
	}
}

fpl_extern void EditorPreRender(RenderState *renderState, const Editor *editor, const Input *input) {
	const GameAssets *assets = editor->assets;
	const World *world = editor->world;
	const Map *map = &world->map;
	const Entity *player = &world->entities.player;

	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);

	Vec2i mapSize = V2iInit(map->width, map->height);
	Vec2f mapArea = V2fHadamard(TileSize, V2fInit((float)mapSize.w, (float)mapSize.h));
	Vec2f mapOrigin = MapTileCoordsToWorld(map, map->origin);

	Vec2f gridSize = mapArea;
	Vec2f gridOrigin = mapOrigin;
	int gridTileCountX = map->width;
	int gridTileCountY = map->height;

	// Editor tile grid
	for (int i = 0; i <= gridTileCountX; ++i) {
		float xoffset = i * TileWidth;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(xoffset, 0));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(xoffset, gridSize.y));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}
	for (int i = 0; i <= gridTileCountY; ++i) {
		float yoffset = i * TileHeight;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(0, yoffset));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(gridSize.x, yoffset));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}
}

fpl_extern void EditorPostRender(RenderState *renderState, const Editor *editor, const Input *input) {
	fpl_localvar char charBuffer[100] = fplZeroInit;

	const GameAssets *assets = editor->assets;
	const World *world = editor->world;
	const Map *map = &world->map;
	const Entity *player = &world->entities.player;

	Vec4f invalidTileColor = V4fInit(1.0f, 0.1f, 0.2f, 1.0f);
	Vec4f placeTileColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);

	AABB2f playerCurrentBounds = EntityGetCurrentBounds(player);

	// Editor mouse tile
	bool drawEditorMouseMouseTile = true;
	if (drawEditorMouseMouseTile) {
		const FontAsset *font = &assets->consoleFont;
		fplAssert(font != fpl_null);
		float fontHeight = 6.0f;

		Vec2i mouseTilePos = MapWorldCoordsToTile(map, editor->mouseWorldPos);
		Vec2f mouseWorldPos = MapTileCoordsToWorld(map, mouseTilePos);

		Tile tile = MapGetTile(map, mouseTilePos);

		uint32_t mouseTileId = tile.id;

		AABB2f tileBounds = AABB2fInitFromBottomLeft(mouseWorldPos, TileSize);

		bool isOverlap = AABB2fIsOverlap(&tileBounds, &playerCurrentBounds);

		Vec4f tileColor = isOverlap ? invalidTileColor : placeTileColor;

		PushRectangle(renderState, mouseWorldPos, TileSize, tileColor, false, 1.0f);

		fplStringFormat(charBuffer, fplArrayCount(charBuffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
		PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, mouseWorldPos, fontHeight, 1.0f, 1.0f, V4fInit(1, 0, 1, 1));
		fplStringFormat(charBuffer, fplArrayCount(charBuffer), "%u", mouseTileId);
		PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fAdd(mouseWorldPos, TileRadius), fontHeight, 0.0f, 0.0f, V4fInit(0, 1, 1, 1));
	}
}

fpl_extern void EditorOSDRender(RenderState *renderState, const Editor *editor, const Input *input) {
	fpl_localvar char charBuffer[100] = fplZeroInit;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	const GameAssets *assets = editor->assets;
	const World *world = editor->world;
	const Map *map = &world->map;
	const Entity *player = &world->entities.player;

	const FontAsset *font = &assets->consoleFont;
	fplAssert(font != fpl_null);
	float fontHeight = 8.0f;

	Vec4f textColor = V4fInit(1, 1, 1, 1);
	Vec4f blackColor = V4fInit(0, 0, 0, 1);
	Vec2f blockPos = V2fInit(-w, -h + fontHeight);

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Drawing: %d x %d, Active: %s, Tile: %d", editor->drawTilePos.x, editor->drawTilePos.y, editor->isDrawing ? "yes" : "no", editor->drawTile);
	PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Mouse: %.04f x %.04f, Down: %s", editor->mouseWorldPos.x, editor->mouseWorldPos.y, input->mouse.left.endedDown ? "yes" : "no");
	PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	PushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));
}