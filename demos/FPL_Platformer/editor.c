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
	} else {
		if (MapResizeToTilePos(map, tilePos)) {
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
	editor->mode = EditorMode_None;
	editor->isDrawing = false;
	editor->isPanning = false;

	if (!CameraInit(&editor->camera, CameraEditorID, V2fInit(0.0f, 0.0f), 1.0f, world->viewRadius)) {
		return false;
	}

	return true;
}

fpl_extern void EditorInput(Editor *editor, const Input *input) {
	fplAssert(editor->mode != EditorMode_None);

	World *world = editor->world;
	Map *map = &world->map;
	Entity *player = &world->entities.player;
	Vec2i mouseTilePos = MapWorldCoordsToTile(map, editor->mouseWorldPos);
	Camera *camera = &editor->camera;

	const worldWidth = world->radius.w * 2.0f;

	const Controller *controller = (input->defaultControllerIndex == -1) ? &input->controllers[0] : &input->controllers[input->defaultControllerIndex];

	if (F32Abs(input->mouse.wheelDelta) > 0.0f) {
		float zoomStep = 1.1f;
		float oldZoom = camera->transform[0].scale;
		float newZoom = oldZoom * (input->mouse.wheelDelta > 0 ? zoomStep : 1.0f / zoomStep);
		newZoom = F32Clamp(newZoom, EDITOR_CAMERA_MIN_ZOOM, EDITOR_CAMERA_MAX_ZOOM);
		if (editor->mode == EditorMode_Full) {
			Vec2f targetPos = V2fNegate(editor->mouseWorldPos);
			CameraZoomToPosition(camera, oldZoom, newZoom, worldWidth, targetPos);
		} else if (editor->mode == EditorMode_Live) {
			CameraSetScale(camera, newZoom, false);
		}
	}

	if (ButtonIsDown(input->mouse.left)) {
		bool actionIsDown = ButtonIsDown(controller->actionDown);
		if (!actionIsDown) {
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
			if (!editor->isPanning) {
				if (editor->mode == EditorMode_Full) {
					editor->isPanning = true;
					editor->panStartPos = editor->mouseWorldPos;
				}
			} else {
				Vec2f delta = V2fSub(editor->mouseWorldPos, editor->panStartPos);
				float panningStrength = 0.5f;
				editor->camera.transform[0].offset = V2fAddMultScalar(editor->camera.transform[0].offset, delta, -panningStrength);
				editor->panStartPos = editor->mouseWorldPos;
			}
		}
	} else {
		if (editor->isDrawing) {
			editor->drawTile = UINT32_MAX;
			editor->drawTilePos = mouseTilePos;
			editor->isDrawing = false;
		}
		if (editor->isPanning || editor->mode != EditorMode_Full) {
			editor->isPanning = false;
			editor->panStartPos = editor->mouseWorldPos;
		}
	}

	if (ButtonWasPressed(input->mouse.right)) {
		if (MapIsTileInside(map, mouseTilePos)) {
			Tile tile = MapGetTile(map, mouseTilePos);
			uint8_t mask = tile.areaMask & 0xFF;
			char tmp[9] = fplZeroInit;
			for (int i = 0; i < 8; ++i) {
				tmp[i] = (mask & (1 << (7 - i))) ? '1' : '0';
			}
			fplDebugFormatOut("0b%s\n", tmp);
		}
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
		RenderPushLine(renderState, a, b, gridColor, 1.0f);
	}
	for (int i = 0; i <= gridTileCountY; ++i) {
		float yoffset = i * TileHeight;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(0, yoffset));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(gridSize.x, yoffset));
		RenderPushLine(renderState, a, b, gridColor, 1.0f);
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

		TileAreaMask areaMask = tile.areaMask;

		AABB2f tileBounds = AABB2fInitFromBottomLeft(mouseWorldPos, TileSize);

		bool isOverlap = AABB2fIsOverlap(&tileBounds, &playerCurrentBounds);

		Vec4f tileColor = isOverlap ? invalidTileColor : placeTileColor;

		RenderPushRectangle(renderState, mouseWorldPos, TileSize, tileColor, false, 1.0f);

		fplStringFormat(charBuffer, fplArrayCount(charBuffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
		RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, mouseWorldPos, fontHeight, 1.0f, 1.0f, V4fInit(1, 0, 1, 1));
		fplStringFormat(charBuffer, fplArrayCount(charBuffer), "%u", mouseTileId);
		RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fAdd(mouseWorldPos, TileRadius), fontHeight, 0.0f, 0.0f, V4fInit(0, 1, 1, 1));
		fplStringFormat(charBuffer, fplArrayCount(charBuffer), "%u", areaMask);
		RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fAdd(mouseWorldPos, TileSize), fontHeight, 0.0f, 0.0f, V4fInit(0, 1, 1, 1));
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
	const Camera *camera = &editor->camera;

	const FontAsset *font = &assets->consoleFont;
	fplAssert(font != fpl_null);
	float fontHeight = 8.0f;

	Vec4f textColor = V4fInit(1, 1, 1, 1);
	Vec4f blackColor = V4fInit(0, 0, 0, 1);
	Vec2f blockPos = V2fInit(-w, -h + fontHeight);

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Drawing: %d x %d, Active: %s, Tile: %d", editor->drawTilePos.x, editor->drawTilePos.y, editor->isDrawing ? "yes" : "no", editor->drawTile);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Mouse: %.04f x %.04f, Down: %s", editor->mouseWorldPos.x, editor->mouseWorldPos.y, input->mouse.left.endedDown ? "yes" : "no");
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Player: %.04f x %.04f", player->transform[0].pos.x, player->transform[0].pos.y);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));

	fplStringFormat(charBuffer, fplArrayCount(charBuffer), "Camera: %.04f x %.04f", camera->transform[0].offset.x, camera->transform[0].offset.y);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
	RenderPushText(renderState, charBuffer, fplGetStringLength(charBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	blockPos = V2fAdd(blockPos, V2fInit(0, fontHeight));
}