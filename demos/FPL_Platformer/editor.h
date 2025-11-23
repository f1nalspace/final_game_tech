#ifndef EDITOR_H
#define EDITOR_H

// Dependencies
#include <final_platform_layer.h>
#include <final_math.h>
#include <final_memory.h> // fmemMemoryBlock

// Game headers
#include "map.h"
#include "physics.h"
#include "entity.h"
#include "world.h"
#include "assets.h"

// Size of the editor transient memory block
#define EDITOR_MEMORY_TRANSIENT_SIZE fplMegaBytes(2)

// Size of the editor persistent memory block
#define EDITOR_MEMORY_PERSISTENT_SIZE fplMegaBytes(8)

#define EDITOR_CAMERA_MIN_ZOOM 0.1f
#define EDITOR_CAMERA_MAX_ZOOM 2.0f

typedef enum EditorMode {
	EditorMode_None = 0,
	EditorMode_Live,
	EditorMode_Full,
} EditorMode;

typedef struct Editor {
	Camera camera;

	fmemMemoryBlock transientMemory;
	fmemMemoryBlock persistentMemory;

	Viewport viewport;

	Vec2f mouseWorldPos;
	Vec2i drawTilePos;

	GameAssets *assets;
	World *world;

	EditorMode mode;

	uint32_t drawTile;
	bool isDrawing;
} Editor;

fpl_extern bool EditorInit(fmemMemoryBlock *gameMemory, Editor *editor, GameAssets *assets, World *world);

fpl_extern void EditorInput(Editor *editor, const Input *input);

fpl_extern void EditorPreRender(RenderState *renderState, const Editor *editor, const Input *input);
fpl_extern void EditorPostRender(RenderState *renderState, const Editor *editor, const Input *input);
fpl_extern void EditorOSDRender(RenderState *renderState, const Editor *editor, const Input *input);

#endif // EDITOR_H
