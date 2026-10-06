/*
Name:
	FUI_Framework

Description:
	The same interactive demo as FUI_Test, built on the Final Framework through final_ui_adapter.h.

	FUI_Test hand-rolls every bridge the library needs: an input bridge over the event queue and the polling
	api, a font baked with stb_truetype, a clipboard pair and a legacy OpenGL backend. That is what using
	final_ui.h from scratch looks like, and it is worth reading first.

	This demo throws all of it away. It is a Final Framework game - final_gameplatform.h drives the loop,
	final_render.h collects the draw commands, final_assets.h loads the font, final_memory.h owns the memory -
	and final_ui_adapter.h is the ONE file that knows both worlds:

	  fuiInputFromFinalGame       the platform's Input   -> fuiInput
	  fuiFontFromFontAsset        a loaded FontAsset     -> fuiFont
	  fuiPlatformFromFPL          FPL's clipboard        -> fuiPlatform
	  fuiAllocatorFromMemoryBlock the game memory block  -> fuiAllocator
	  fuiRenderDrawData           fuiGetDrawData         -> RenderState

	Everything BETWEEN those five calls - the whole interface, from the menu bar down to the file browser
	dialog - is what FUI_Test builds, line for line. That is the point of the exercise: the library does not
	know which host it runs on, and interface code does not change when the host does.

	Two things ARE different, and both are the framework's doing rather than the library's:

	  - The interface is built inside GameRender, not GameUpdate. An immediate mode interface builds and draws
	    in one pass, and the render state it draws into is reset once per RENDERED frame - so a fixed timestep
	    update, which may run several times or not at all between two frames, is the wrong place for it.
	  - A texture is not ready when it is asked for. RenderPushTexture QUEUES an upload that the pipeline
	    performs at the end of the frame, so the font and the icon sheet are wired to the interface on the
	    first frame their handles are real, rather than in GameInit. See WireQueuedTextures below.

Requirements:
	- C99 compiler
	- OpenGL 1.1 (the render pipeline is fixed function)
	- Final Framework

Build (from the repository root):
	cmake -S demos/FUI_Framework -B build/fui_framework && cmake --build build/fui_framework

Changelog:
	## 2026-08-25
	- Renamed from FUI_Adapter_FPL: the old name claimed FPL as the distinction, but FUI_Test is an FPL program too - what actually differs is the HOST, which is the Final Framework
	- New: a teal accent, a backdrop mixed toward it and the host named in the status bar, so the two demos are told apart on sight rather than by their title bars

	## 2026-08-24
	- Initial version, ported from FUI_Test onto final_ui_adapter.h

License:
	MIT License, Copyright (c) 2017-2026 Torsten Spaete
*/


#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#include <final_game.h>

// The embedded TrueType faces, so the demo is a single executable with no asset beside it.
#include <final_fonts.h>

#define FUI_IMPLEMENTATION
#include <final_ui.h>

// The one file that knows both worlds. Every call below reaches the library THROUGH it.
#define FINAL_UI_ADAPTER_IMPLEMENTATION
#include <final_ui_adapter.h>

#include <stdarg.h>
#include <string.h>

#define DEMO_WINDOW_TITLE "final_ui.h demo (Final Framework, through final_ui_adapter.h)"
#define DEMO_LOG_CATEGORY "FUI_Framework"

// What tells this demo from FUI_Test at a glance, in a screenshot with no title bar in it: a teal accent
// where FUI_Test keeps the theme's default amber, a backdrop mixed toward the same hue, and the host named
// in the status bar. Nothing between fuiBeginFrame and fuiEndFrame changes - the interface is still FUI_Test's,
// line for line, which is the point of the demo.
#define DEMO_HOST_LABEL "Final Framework"
static const fuiColor g_demoAccentColor = FUI_COLOR(0.30f, 0.78f, 0.74f, 1.0f);

// Loaded once, above the largest text on screen, so every size drawn is a reduction of the atlas. The range
// is the one FUI_Test bakes: printable ASCII, which is every character this demo puts on screen.
#define DEMO_FONT_PIXEL_HEIGHT 34.0f
#define DEMO_FONT_ATLAS_SIDE 512u
#define DEMO_FONT_FIRST_CODEPOINT 32u
#define DEMO_FONT_LAST_CODEPOINT 126u

#define DEMO_ROW_HEIGHT 26.0f
#define DEMO_STATUS_MESSAGE_MAX 160
#define DEMO_NAME_FIELD_MAX 64
#define DEMO_LIST_ROW_COUNT 40

// The table panel. Two hundred rows, because a list view is a widget for more rows than anybody wants to
// scroll past - which is also what makes sorting worth having.
#define DEMO_TABLE_ROW_COUNT 200
#define DEMO_TABLE_COLUMN_COUNT 4
#define DEMO_TABLE_NAME_MAX 32
#define DEMO_TABLE_CELL_COUNT (DEMO_TABLE_ROW_COUNT * DEMO_TABLE_COLUMN_COUNT)

// The icon sheet the demo draws itself: four square cells in a row, one channel of coverage. No asset to
// ship, and a worked example of what fuiListIcons wants.
#define DEMO_ICON_CELL_SIDE 32u
// Eight cells: the four entity kinds the table draws, and behind them the four the project tree draws. Eight
// keeps the sheet 256 wide and so a power of two in both axes, which is what the oldest backends want.
#define DEMO_ICON_CELL_COUNT 8u
#define DEMO_ICON_CELL_FOLDER_SHUT 4
#define DEMO_ICON_CELL_FOLDER_OPEN 5
#define DEMO_ICON_CELL_FILE 6
#define DEMO_ICON_CELL_LEVEL 7
#define DEMO_ICON_SHEET_WIDTH (DEMO_ICON_CELL_SIDE * DEMO_ICON_CELL_COUNT)
#define DEMO_ICON_SHEET_HEIGHT DEMO_ICON_CELL_SIDE

// The sheet goes up with FOUR channels rather than one - white, with the coverage in alpha. A single channel
// upload becomes a GL_ALPHA texture, whose rgb reads as zero, and a sprite modulated against that draws
// nothing but black. That is the same expansion FUI_Test's legacy backend does inside its own atlas upload.
#define DEMO_ICON_TEXEL_CHANNELS 4u
#define DEMO_ICON_TEXEL_COUNT (DEMO_ICON_SHEET_WIDTH * DEMO_ICON_SHEET_HEIGHT)
#define DEMO_ICON_BYTE_COUNT (DEMO_ICON_TEXEL_COUNT * DEMO_ICON_TEXEL_CHANNELS)

#define DEMO_TREE_NODE_MAX 64
#define DEMO_TREE_ID "projecttree"
//! The file the Find button digs out, named rather than indexed so the tree table can be edited freely
#define DEMO_TREE_BURIED_FILE "water-01.wav"

// The framework leaves the window at whatever FPL defaults to, which is not wide enough for an explorer
// column beside the panels that were already there. Asked for in GameInit.
#define DEMO_WINDOW_WIDTH 1560u
#define DEMO_WINDOW_HEIGHT 800u

// ----------------------------------------------------------------------------
// What the demo is about
// ----------------------------------------------------------------------------

typedef struct DemoState {
	bool isRunning;

	// Panels the View menu opens and closes. A closable panel writes its own flag when its X is clicked.
	bool showWidgetsPanel;
	bool showPickerPanel;
	bool showListPanel;
	bool showTablePanel;
	bool showTreePanel;
	bool showTooltips;

	// Values the widgets edit. The library holds NONE of these: an immediate mode widget reads the
	// caller's variable, draws it, and writes it back when it changes.
	bool snapToGrid;
	bool showGrid;
	int32_t toolSelection;
	float zoom;
	float gridSize;
	float dragValue;
	fuiColor tint;
	char nameField[DEMO_NAME_FIELD_MAX];
	int32_t selectedRow;

	// What the command table has been asked to do.
	int32_t newCount;
	int32_t saveCount;

	// What the dialogs edit. A dialog is opened by identifier and then BUILT every frame: the build
	// answers nothing at all while it is closed, so there is no flag here saying which one is up.
	char renameField[DEMO_NAME_FIELD_MAX];
	fuiColor dialogColor;
	bool colorLiveUpdate;
	//! Which listing the file browser is showing, since the LIBRARY reads no directory of its own
	int32_t browserFolder;
	int32_t browserSelection;
	char browserName[DEMO_NAME_FIELD_MAX];

	// The table panel: the rows themselves, which the DEMO owns - the library reads the strings and never
	// copies one - plus what the user has done to the view of them.
	char tableNames[DEMO_TABLE_ROW_COUNT][DEMO_TABLE_NAME_MAX];
	char tableWeights[DEMO_TABLE_ROW_COUNT][DEMO_TABLE_NAME_MAX];
	const char *tableCells[DEMO_TABLE_CELL_COUNT];
	const char *tableRowButtons[DEMO_TABLE_ROW_COUNT];
	int32_t tableIconForRow[DEMO_TABLE_ROW_COUNT];
	int32_t tableSelection;
	//! Which row is sounding, so its button reads Stop while every other one reads Play
	int32_t tablePlayingRow;
	//! The icon sheet, drawn into an alpha bitmap at startup and uploaded like the font atlas
	fuiTextureId iconSheet;
	fuiVec2 iconSheetSize;
	//! Which image the preview tab is showing, and how
	int32_t previewScaleMode;
	bool previewIsMirrored;
	bool previewIsTurned;

	// The project tree. The nodes and their flags are the DEMO's, exactly like the table's rows: the library
	// reads them, writes a flag when an expander is clicked, and copies nothing.
	fuiTreeNode treeNodes[DEMO_TREE_NODE_MAX];
	bool treeIsExpanded[DEMO_TREE_NODE_MAX];
	int32_t treeIconForNode[DEMO_TREE_NODE_MAX];
	//! One tint per node. This sheet is a single channel of coverage, so a folder and a file are told apart by
	//! the COLOR they are modulated with rather than by the artwork - which is what tintForRow is for
	fuiColor treeTintForNode[DEMO_TREE_NODE_MAX];
	int32_t treeNodeCount;
	int32_t treeSelection;
	//! Which node the context menu is about, which is the row the right button went down on and not the selection
	int32_t treeContextNode;
	bool treeShowsGuides;
	bool treeUsesIcons;
	bool treeKeyboardIsOn;

	char statusMessage[DEMO_STATUS_MESSAGE_MAX];
	float framesPerSecond;

	/*
		Whether the interface owned the cursor at the END of the previous frame.

		In one pass mode fuiWantsMouse is only final once the frame is finished: it becomes true as the
		widget under the cursor is built, and an open menu forces it in fuiEndFrame. Anything asking the
		question WHILE the interface is being built - the world deciding whether a click was meant for it,
		a status line reporting the answer - therefore has to ask about the previous frame.

		A host that cannot live with that one frame of lag builds twice instead, fuiPass_Interact before it
		reads input and fuiPass_Draw when it renders, and then the answer is current inside the same frame.
	*/
	bool uiOwnedTheMouseLastFrame;
} DemoState;

/*
	What the game platform hands back and forth.

	The DemoState above is the interface's own and is untouched from FUI_Test. Everything beside it here is
	what the framework needs to draw it: the loaded font, the interface context, and the icon sheet as a
	pipeline texture rather than a raw OpenGL name.
*/
typedef struct GameState {
	DemoState demo;

	//! The interface font, and the atlas texture the pipeline uploads for it
	FontAsset uiFont;
	//! The font as the LIBRARY sees it. Its address is what fuiInit was given, so it is refreshed in place
	fuiFont font;
	fuiContext ui;

	//! The icon sheet's texels, which outlive the RenderPushTexture call because the upload is deferred
	unsigned char iconTexels[DEMO_ICON_BYTE_COUNT];
	TextureHandle iconTexture;

	//! False until the queued uploads have handles and the interface has been pointed at them
	bool areTexturesWired;
} GameState;

static void DemoSay(DemoState *demo, const char *message) {
	fplCopyString(message, demo->statusMessage, fplArrayCount(demo->statusMessage));
}

//! The same line, with whatever made it worth saying folded into it
static void DemoSayFormat(DemoState *demo, const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	fplStringFormatArgs(demo->statusMessage, fplArrayCount(demo->statusMessage), format, arguments);
	va_end(arguments);
}

// The vocabulary the two hundred generated rows are built out of. A row's kind decides its icon, so the
// four cells of the sheet and the four kinds line up.
static const char *const g_demoTableKinds[] = { "Solid", "Prop", "Trigger", "Light" };

/*
	The project tree, written out in PREORDER - which is the order the rows come out in when everything is open,
	and the order a folder read recursively falls out in anyway.

	It is a table and not a real directory on purpose: the library reads no directory, and neither does the demo,
	so what this panel shows is the same on every machine it is run on.
*/
typedef struct DemoTreeEntry {
	//! What the row says
	const char *label;
	//! How deep it sits, zero being a root
	int32_t depth;
	//! Which sheet cell a LEAF draws. A folder says minus one and takes the shut or the open folder cell from
	//! whether it happens to be folded open, which is the demo's business and not the library's
	int32_t leafIconCell;
} DemoTreeEntry;

#define DEMO_TREE_FOLDER (-1)

static const DemoTreeEntry g_demoTreeEntries[] = {
	{ "assets",                    0, DEMO_TREE_FOLDER },
	{ "fonts",                     1, DEMO_TREE_FOLDER },
	{ "bitstream-vera.ttf",        2, DEMO_ICON_CELL_FILE },
	{ "sulphur-point.ttf",         2, DEMO_ICON_CELL_FILE },
	{ "sprites",                   1, DEMO_TREE_FOLDER },
	{ "hero",                      2, DEMO_TREE_FOLDER },
	{ "idle.png",                  3, DEMO_ICON_CELL_FILE },
	{ "run.png",                   3, DEMO_ICON_CELL_FILE },
	{ "fall.png",                  3, DEMO_ICON_CELL_FILE },
	{ "tiles",                     2, DEMO_TREE_FOLDER },
	{ "grass.png",                 3, DEMO_ICON_CELL_FILE },
	{ "stone.png",                 3, DEMO_ICON_CELL_FILE },
	{ "interface",                 2, DEMO_TREE_FOLDER },
	{ "cursor.png",                3, DEMO_ICON_CELL_FILE },
	{ "icons.png",                 3, DEMO_ICON_CELL_FILE },
	{ "audio",                     1, DEMO_TREE_FOLDER },
	{ "music",                     2, DEMO_TREE_FOLDER },
	{ "the-long-dusk.ogg",         3, DEMO_ICON_CELL_FILE },
	{ "steps",                     2, DEMO_TREE_FOLDER },
	{ "gravel-01.wav",             3, DEMO_ICON_CELL_FILE },
	{ "gravel-02.wav",             3, DEMO_ICON_CELL_FILE },
	{ "water-01.wav",              3, DEMO_ICON_CELL_FILE },
	{ "levels",                    0, DEMO_TREE_FOLDER },
	{ "gardens-of-ash.lvl",        1, DEMO_ICON_CELL_LEVEL },
	{ "the-drowned-mill.lvl",      1, DEMO_ICON_CELL_LEVEL },
	{ "the-quiet-observatory.lvl", 1, DEMO_ICON_CELL_LEVEL },
	{ "drafts",                    1, DEMO_TREE_FOLDER },
	{ "untitled-7.lvl",            2, DEMO_ICON_CELL_LEVEL },
	{ "untitled-8.lvl",            2, DEMO_ICON_CELL_LEVEL },
	{ "scripts",                   0, DEMO_TREE_FOLDER },
	{ "boot.lua",                  1, DEMO_ICON_CELL_FILE },
	{ "dialogue.lua",              1, DEMO_ICON_CELL_FILE },
	{ "weather.lua",               1, DEMO_ICON_CELL_FILE },
	{ "tools",                     0, DEMO_TREE_FOLDER },
	{ "bake-atlas.sh",             1, DEMO_ICON_CELL_FILE },
	{ "pack-levels.sh",            1, DEMO_ICON_CELL_FILE },
	{ "readme.txt",                0, DEMO_ICON_CELL_FILE },
	{ "license.txt",               0, DEMO_ICON_CELL_FILE },
};

/*
	The icon sheet, drawn by the demo into one channel of coverage - a square, a triangle, a ring and a diamond
	for the entity kinds, then a shut folder, an open one, a file and a level file for the project tree. It goes
	up through the same call the font atlas does, because to this backend an atlas IS a coverage bitmap, and a
	list icon and a glyph ask exactly the same thing of it.

	One channel carries no colour, so what tells a folder from a file here is the TINT each row is modulated
	with - fuiListIcons.tintForRow - rather than the artwork.

	The library never learns what a cell MEANS. It draws cell N for the row whose entry says N, and which
	cell a row gets is the table below.
*/
static void DemoDrawIconSheet(unsigned char *coveragePixels) {
	const int32_t cellSide = (int32_t)DEMO_ICON_CELL_SIDE;
	const int32_t sheetWidth = (int32_t)DEMO_ICON_SHEET_WIDTH;
	const float centre = (float)cellSide * 0.5f;
	const float outerRadius = (float)cellSide * 0.36f;
	const float innerRadius = (float)cellSide * 0.24f;
	// Compared as squares, so the sheet needs no square root and the demo no math header of its own.
	const float outerRadiusSquared = outerRadius * outerRadius;
	const float innerRadiusSquared = innerRadius * innerRadius;
	const int32_t squareInset = cellSide / 5;

	// The four shapes the project tree is drawn with, measured off the cell so they scale with it. A folder is
	// a tab over a body, and the open one is the same folder with its lid lifted clear of it.
	const int32_t folderLeft = cellSide / 8;
	const int32_t folderRight = cellSide - cellSide / 8;
	const int32_t folderTabRight = folderLeft + cellSide / 3;
	const int32_t folderTabTop = cellSide / 5;
	const int32_t folderTabBottom = folderTabTop + cellSide / 8;
	const int32_t folderBottom = cellSide - cellSide / 6;
	const int32_t folderLidBottom = folderTabBottom + cellSide / 10;
	const int32_t folderOpenBodyTop = folderLidBottom + cellSide / 12;
	const int32_t folderOpenBodyLeft = folderLeft + cellSide / 10;

	const int32_t fileLeft = cellSide / 4;
	const int32_t fileRight = cellSide - cellSide / 5;
	const int32_t fileTop = cellSide / 6;
	const int32_t fileBottom = cellSide - cellSide / 6;
	const int32_t fileFoldSize = cellSide / 4;
	const int32_t fileFoldLeft = fileRight - fileFoldSize;
	const int32_t fileFoldBottom = fileTop + fileFoldSize;
	const int32_t fileLineLeft = fileLeft + cellSide / 10;
	const int32_t fileLineRight = fileRight - cellSide / 10;
	const int32_t fileFirstLineTop = fileTop + fileFoldSize + cellSide / 12;
	const int32_t fileLineSpacing = cellSide / 7;
	const int32_t fileLineThickness = cellSide / 16 + 1;
	const int32_t fileLineCount = 3;

	memset(coveragePixels, 0, (size_t)sheetWidth * (size_t)cellSide);
	for(int32_t cellIndex = 0; cellIndex < (int32_t)DEMO_ICON_CELL_COUNT; ++cellIndex) {
		for(int32_t y = 0; y < cellSide; ++y) {
			for(int32_t x = 0; x < cellSide; ++x) {
				float offsetX = ((float)x + 0.5f) - centre;
				float offsetY = ((float)y + 0.5f) - centre;
				float distanceSquared = offsetX * offsetX + offsetY * offsetY;
				bool isTheFolderTab = (x >= folderLeft) && (x < folderTabRight) && (y >= folderTabTop) && (y < folderTabBottom);
				bool isTheFileSheet = (x >= fileLeft) && (x < fileRight) && (y >= fileTop) && (y < fileBottom);
				bool isTheFileFold = (x >= fileFoldLeft) && (y < fileFoldBottom) && (((x - fileFoldLeft) + (fileFoldBottom - y)) > fileFoldSize);
				bool isInk = false;
				switch(cellIndex) {
					case 0: // a solid block
						isInk = (x >= squareInset) && (x < cellSide - squareInset) && (y >= squareInset) && (y < cellSide - squareInset);
						break;
					case 1: // a triangle standing on its base
						isInk = (y >= squareInset) && (y < cellSide - squareInset) && (offsetX < 0.0f ? -offsetX : offsetX) <= (float)(y - squareInset) * 0.5f;
						break;
					case 2: // a ring
						isInk = (distanceSquared <= outerRadiusSquared) && (distanceSquared >= innerRadiusSquared);
						break;
					case 3: // a diamond
						isInk = ((offsetX < 0.0f ? -offsetX : offsetX) + (offsetY < 0.0f ? -offsetY : offsetY)) <= outerRadius;
						break;
					case DEMO_ICON_CELL_FOLDER_SHUT:
					{
						bool isBody = (x >= folderLeft) && (x < folderRight) && (y >= folderTabBottom) && (y < folderBottom);
						isInk = isTheFolderTab || isBody;
					} break;
					case DEMO_ICON_CELL_FOLDER_OPEN:
					{
						// The same tab, a lid where the body's top edge was, and the body itself dropped and
						// pushed right - a folder with its lid taken off rather than a different folder.
						bool isLid = (x >= folderLeft) && (x < folderRight) && (y >= folderTabBottom) && (y < folderLidBottom);
						bool isBody = (x >= folderOpenBodyLeft) && (x < folderRight) && (y >= folderOpenBodyTop) && (y < folderBottom);
						isInk = isTheFolderTab || isLid || isBody;
					} break;
					case DEMO_ICON_CELL_FILE:
						isInk = isTheFileSheet && !isTheFileFold;
						break;
					default: // the same sheet with lines written on it, which is what a level file gets
					{
						bool isBetweenTheLines = false;
						for(int32_t lineIndex = 0; lineIndex < fileLineCount; ++lineIndex) {
							int32_t lineTop = fileFirstLineTop + lineIndex * fileLineSpacing;
							bool isOnThisLine = (y >= lineTop) && (y < lineTop + fileLineThickness) && (x >= fileLineLeft) && (x < fileLineRight);
							if(isOnThisLine) {
								isBetweenTheLines = true;
							}
						}
						isInk = isTheFileSheet && !isTheFileFold && !isBetweenTheLines;
					} break;
				}
				if(isInk) {
					int32_t sheetX = cellIndex * cellSide + x;
					coveragePixels[(size_t)y * (size_t)sheetWidth + (size_t)sheetX] = 255;
				}
			}
		}
	}
}

// What each kind of tree row is modulated with, since one channel of coverage carries no colour of its own.
static const fuiColor g_demoTreeFolderTint = FUI_COLOR(0.94f, 0.76f, 0.38f, 1.0f);
static const fuiColor g_demoTreeFileTint = FUI_COLOR(0.78f, 0.83f, 0.90f, 1.0f);
static const fuiColor g_demoTreeLevelTint = FUI_COLOR(0.62f, 0.82f, 0.92f, 1.0f);

//! Every node's icon and its colour, worked out from what it IS and from whether it is folded open right now
/*
	Called every frame before the tree is built, which is cheap for a table this size and always right. It is
	also the whole point of the icon table belonging to the caller: the library never learns that cell four is a
	shut folder and cell five an open one, it just draws the cell the row asks for in the colour the row asks for.
*/
static void DemoRefreshTreeIcons(DemoState *demo) {
	for(int32_t nodeIndex = 0; nodeIndex < demo->treeNodeCount; ++nodeIndex) {
		int32_t leafIconCell = g_demoTreeEntries[nodeIndex].leafIconCell;
		bool isFolder = (leafIconCell == DEMO_TREE_FOLDER);
		if(isFolder) {
			bool isOpen = demo->treeIsExpanded[nodeIndex];
			demo->treeIconForNode[nodeIndex] = isOpen ? DEMO_ICON_CELL_FOLDER_OPEN : DEMO_ICON_CELL_FOLDER_SHUT;
			demo->treeTintForNode[nodeIndex] = g_demoTreeFolderTint;
			continue;
		}
		demo->treeIconForNode[nodeIndex] = leafIconCell;
		bool isALevel = (leafIconCell == DEMO_ICON_CELL_LEVEL);
		demo->treeTintForNode[nodeIndex] = isALevel ? g_demoTreeLevelTint : g_demoTreeFileTint;
	}
}

//! Which node carries a label, so the demo can name one instead of counting rows to it
static int32_t DemoFindTreeNode(const DemoState *demo, const char *label) {
	for(int32_t nodeIndex = 0; nodeIndex < demo->treeNodeCount; ++nodeIndex) {
		if(fplIsStringEqual(demo->treeNodes[nodeIndex].label, label)) {
			return(nodeIndex);
		}
	}
	return(-1);
}

//! Writes out the path of a node, which is what the panel's footer says
static void DemoTreePathOf(const DemoState *demo, const int32_t nodeIndex, char *buffer, const size_t capacity) {
	buffer[0] = 0;
	bool nodeIsInside = (nodeIndex >= 0) && (nodeIndex < demo->treeNodeCount);
	if(!nodeIsInside) {
		return;
	}
	// Ancestors can only be walked UPWARDS out of a preorder array, so the chain is collected from the node
	// towards its root and then written out the other way round.
	int32_t chain[FUI_MAX_TREE_DEPTH];
	int32_t chainLength = 0;
	int32_t walkIndex = nodeIndex;
	while(walkIndex >= 0 && chainLength < (int32_t)fplArrayCount(chain)) {
		chain[chainLength] = walkIndex;
		chainLength += 1;
		walkIndex = fuiTreeParentOf(demo->treeNodes, demo->treeNodeCount, walkIndex);
	}
	for(int32_t step = chainLength - 1; step >= 0; --step) {
		int32_t pathNode = chain[step];
		const char *pathLabel = demo->treeNodes[pathNode].label;
		fplStringAppend("/", buffer, capacity);
		fplStringAppend(pathLabel, buffer, capacity);
	}
}

//! Copies the table into the nodes, works out the subtree sizes once, and opens the roots
static void DemoBuildTree(DemoState *demo) {
	int32_t entryCount = (int32_t)fplArrayCount(g_demoTreeEntries);
	if(entryCount > DEMO_TREE_NODE_MAX) {
		entryCount = DEMO_TREE_NODE_MAX;
	}
	for(int32_t nodeIndex = 0; nodeIndex < entryCount; ++nodeIndex) {
		const DemoTreeEntry *entry = &g_demoTreeEntries[nodeIndex];
		demo->treeNodes[nodeIndex].label = entry->label;
		demo->treeNodes[nodeIndex].depth = entry->depth;
		demo->treeIsExpanded[nodeIndex] = false;
	}
	demo->treeNodeCount = entryCount;

	// Once, here, and never per frame. This is what lets a folded node be stepped over in one addition.
	fuiTreeComputeDescendants(demo->treeNodes, demo->treeNodeCount);

	// The roots start open, so the panel opens on something rather than on four shut folders.
	for(int32_t nodeIndex = 0; nodeIndex < demo->treeNodeCount; ++nodeIndex) {
		bool isRoot = (demo->treeNodes[nodeIndex].depth == 0);
		if(isRoot) {
			demo->treeIsExpanded[nodeIndex] = true;
		}
	}
	DemoRefreshTreeIcons(demo);
}

//! Fills the table with rows, and points the flat cell array at them. The strings live in DemoState for as
//! long as the demo does, which is what a list view wants: it reads them and copies nothing.
static void DemoBuildTable(DemoState *demo) {
	const int32_t kindCount = (int32_t)fplArrayCount(g_demoTableKinds);
	for(int32_t rowIndex = 0; rowIndex < DEMO_TABLE_ROW_COUNT; ++rowIndex) {
		int32_t kindIndex = rowIndex % kindCount;
		const char *kind = g_demoTableKinds[kindIndex];
		// Named with the number LAST and unpadded on purpose, so sorting by name shows off the natural
		// compare: "crate 9" belongs before "crate 10" and a plain text sort puts it after.
		fplStringFormat(demo->tableNames[rowIndex], DEMO_TABLE_NAME_MAX, "%s %d", kind, rowIndex + 1);
		// Two decimals, which is the other thing a plain text sort reads wrongly: "10.00" before "9.00".
		float weight = 0.25f + (float)((rowIndex * 37) % 400) * 0.05f;
		fplStringFormat(demo->tableWeights[rowIndex], DEMO_TABLE_NAME_MAX, "%.2f", (double)weight);

		int32_t cellAt = rowIndex * DEMO_TABLE_COLUMN_COUNT;
		demo->tableCells[cellAt + 0] = demo->tableNames[rowIndex];
		demo->tableCells[cellAt + 1] = kind;
		demo->tableCells[cellAt + 2] = demo->tableWeights[rowIndex];
		demo->tableCells[cellAt + 3] = ""; // the button column draws over its own cell
		demo->tableIconForRow[rowIndex] = kindIndex;
		demo->tableRowButtons[rowIndex] = "Play";
	}
	// One row without a button, to show that a row opts out by having no label rather than by a flag.
	demo->tableRowButtons[0] = fpl_null;
}

static void DemoInit(DemoState *demo) {
	fplClearStruct(demo);
	demo->isRunning = true;
	demo->showWidgetsPanel = true;
	demo->showPickerPanel = true;
	demo->showListPanel = true;
	demo->showTablePanel = true;
	demo->showTooltips = true;
	demo->showTreePanel = true;
	demo->tableSelection = -1;
	demo->tablePlayingRow = -1;
	demo->treeSelection = -1;
	demo->treeContextNode = -1;
	demo->treeShowsGuides = true;
	demo->treeUsesIcons = true;
	demo->treeKeyboardIsOn = true;
	DemoBuildTable(demo);
	DemoBuildTree(demo);
	demo->showGrid = true;
	demo->toolSelection = 1;
	demo->zoom = 1.4f;
	demo->gridSize = 16.0f;
	demo->dragValue = 42.0f;
	demo->tint = fuiColorRGBA(0.35f, 0.62f, 0.95f, 1.0f);
	demo->selectedRow = 3;
	demo->dialogColor = fuiColorRGBA(0.92f, 0.55f, 0.20f, 1.0f);
	demo->browserSelection = -1;
	fplCopyString("gardens-of-ash", demo->nameField, fplArrayCount(demo->nameField));
	fplCopyString("gardens-of-ash", demo->renameField, fplArrayCount(demo->renameField));
	fplCopyString("untitled.lvl", demo->browserName, fplArrayCount(demo->browserName));
	DemoSay(demo, "Ready. Try the menus, drag a panel by its title, and press Ctrl+Q to quit.");
}

// ----------------------------------------------------------------------------
// The command table
//
// One row per action: its label, its shortcut, and what to run. The menu rows, the tool strip buttons and
// the keyboard all reference the SAME row, which is why a shortcut cannot drift from the text next to it.
// ----------------------------------------------------------------------------

#define DEMO_COMMAND_NEW 1
#define DEMO_COMMAND_SAVE 2
#define DEMO_COMMAND_QUIT 3

static void DemoInvokeNew(void *userData) {
	DemoState *demo = (DemoState *)userData;
	demo->newCount += 1;
	DemoSay(demo, "New: the command ran, from a menu row, a strip button or Ctrl+N.");
}

static void DemoInvokeSave(void *userData) {
	DemoState *demo = (DemoState *)userData;
	demo->saveCount += 1;
	DemoSay(demo, "Save: nothing is written, but the command really did fire.");
}

static void DemoInvokeQuit(void *userData) {
	DemoState *demo = (DemoState *)userData;
	demo->isRunning = false;
}

static bool DemoSaveIsEnabled(void *userData) {
	DemoState *demo = (DemoState *)userData;
	// Disabled until something has been made, so the demo shows a greyed row and a dead strip button.
	return demo->newCount > 0;
}

static const fuiCommand g_demoCommandRows[] = {
	{ DEMO_COMMAND_NEW, "New", { fuiKey_N, (uint32_t)fuiModifier_Control }, fpl_null, fpl_null, DemoInvokeNew },
	{ DEMO_COMMAND_SAVE, "Save", { fuiKey_S, (uint32_t)fuiModifier_Control }, DemoSaveIsEnabled, fpl_null, DemoInvokeSave },
	{ DEMO_COMMAND_QUIT, "Quit", { fuiKey_Q, (uint32_t)fuiModifier_Control }, fpl_null, fpl_null, DemoInvokeQuit },
};

static const fuiCommandTable g_demoCommandTable = { g_demoCommandRows, (uint32_t)fplArrayCount(g_demoCommandRows) };

// ----------------------------------------------------------------------------
// The interface itself
// ----------------------------------------------------------------------------

static void BuildMenuBar(fuiContext *ui, DemoState *demo, const fuiRect barRect) {
	fuiBeginMenuBar(ui, "menubar", barRect);

	if(fuiBeginMenu(ui, "Demo")) {
		(void)fuiMenuItemCommand(ui, &g_demoCommandTable, DEMO_COMMAND_NEW, demo);
		if(fuiBeginMenu(ui, "Open recent")) {
			if(fuiMenuItem(ui, "gardens-of-ash", fpl_null, true)) {
				DemoSay(demo, "Opened gardens-of-ash from the submenu.");
			}
			if(fuiMenuItem(ui, "the-drowned-mill", fpl_null, true)) {
				DemoSay(demo, "Opened the-drowned-mill from the submenu.");
			}
			if(fuiMenuItem(ui, "nothing here yet", fpl_null, false)) {
				DemoSay(demo, "This row is disabled and can never say this.");
			}
		}
		fuiEndMenu(ui);
		(void)fuiMenuItemCommand(ui, &g_demoCommandTable, DEMO_COMMAND_SAVE, demo);
		fuiMenuSeparator(ui);
		(void)fuiMenuItemCommand(ui, &g_demoCommandTable, DEMO_COMMAND_QUIT, demo);
	}
	fuiEndMenu(ui);

	if(fuiBeginMenu(ui, "View")) {
		if(fuiMenuItemCheck(ui, "Widgets panel", demo->showWidgetsPanel, true)) {
			demo->showWidgetsPanel = !demo->showWidgetsPanel;
		}
		if(fuiMenuItemCheck(ui, "Colour panel", demo->showPickerPanel, true)) {
			demo->showPickerPanel = !demo->showPickerPanel;
		}
		if(fuiMenuItemCheck(ui, "Scrolling list", demo->showListPanel, true)) {
			demo->showListPanel = !demo->showListPanel;
		}
		if(fuiMenuItemCheck(ui, "Entity table", demo->showTablePanel, true)) {
			demo->showTablePanel = !demo->showTablePanel;
		}
		if(fuiMenuItemCheck(ui, "Project tree", demo->showTreePanel, true)) {
			demo->showTreePanel = !demo->showTreePanel;
		}
		fuiMenuSeparator(ui);
		if(fuiMenuItemCheck(ui, "Hover tooltips", demo->showTooltips, true)) {
			demo->showTooltips = !demo->showTooltips;
		}
	}
	fuiEndMenu(ui);

	if(fuiBeginMenu(ui, "Dialogs")) {
		if(fuiMenuItem(ui, "Message box", fpl_null, true)) {
			fuiOpenDialog(ui, "discard");
		}
		if(fuiMenuItem(ui, "Input box", fpl_null, true)) {
			fuiOpenDialog(ui, "rename");
		}
		if(fuiMenuItem(ui, "Colour dialog", fpl_null, true)) {
			fuiOpenDialog(ui, "tint");
		}
		fuiMenuSeparator(ui);
		if(fuiMenuItem(ui, "Save level as...", fpl_null, true)) {
			// Reset the listing, or the browser reopens part way into whatever folder it was left in.
			demo->browserFolder = 0;
			demo->browserSelection = -1;
			fuiOpenDialog(ui, "browse");
		}
	}
	fuiEndMenu(ui);

	if(fuiBeginMenu(ui, "Help")) {
		if(fuiMenuItem(ui, "What am I looking at?", fpl_null, true)) {
			DemoSay(demo, "final_ui.h: one header, no dependencies. This window is a Final Framework game, wired up by final_ui_adapter.h.");
		}
	}
	fuiEndMenu(ui);

	fuiEndMenuBar(ui);
}

static void BuildToolStrip(fuiContext *ui, DemoState *demo, const fuiRect stripRect) {
	fuiBeginToolStrip(ui, "toolstrip", stripRect, fuiAxis_Horizontal);
	if(fuiToolStripToggle(ui, "Select", demo->toolSelection == 0, true)) {
		demo->toolSelection = 0;
	}
	if(fuiToolStripToggle(ui, "Paint", demo->toolSelection == 1, true)) {
		demo->toolSelection = 1;
	}
	if(fuiToolStripToggle(ui, "Erase", demo->toolSelection == 2, true)) {
		demo->toolSelection = 2;
	}
	fuiToolStripSeparator(ui);
	(void)fuiToolStripCommand(ui, &g_demoCommandTable, DEMO_COMMAND_NEW, demo);
	(void)fuiToolStripCommand(ui, &g_demoCommandTable, DEMO_COMMAND_SAVE, demo);
	fuiToolStripSeparator(ui);
	if(fuiToolStripToggle(ui, "Grid", demo->showGrid, true)) {
		demo->showGrid = !demo->showGrid;
	}
	if(fuiToolStripToggle(ui, "Snap", demo->snapToGrid, true)) {
		demo->snapToGrid = !demo->snapToGrid;
	}
	fuiEndToolStrip(ui);
}

//! A tooltip only when the View menu says so, so the menu toggle visibly does something
static void DemoTooltip(fuiContext *ui, const DemoState *demo, const fuiRect rect, const char *text) {
	if(demo->showTooltips) {
		fuiTooltip(ui, rect, text);
	}
}

static void BuildWidgetsPanel(fuiContext *ui, DemoState *demo) {
	if(!demo->showWidgetsPanel) {
		return;
	}
	// Closable: the X in its title bar writes the same flag the View menu toggles.
	if(fuiBeginScrollPanelClosable(ui, "Widgets", fuiDock_None, 24.0f, 110.0f, 360.0f, 470.0f, &demo->showWidgetsPanel)) {
		fuiLabel(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Every widget the library has so far.");
		fuiSeparator(ui, fuiLayoutSlot(ui, 12.0f));

		fuiRect nameRow = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		if(fuiTextInput(ui, nameRow, "name", demo->nameField, (int32_t)fplArrayCount(demo->nameField))) {
			DemoSay(demo, "The name field changed. Ctrl+C and Ctrl+V go through FPL's clipboard.");
		}
		DemoTooltip(ui, demo, nameRow, "Click to focus, then type.\nCtrl+A, Ctrl+C, Ctrl+V and Ctrl+X all work.\nTab moves to the next field.");

		fuiRect buttonRow = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT + 4.0f);
		if(fuiButton(ui, buttonRow, "An ordinary button")) {
			DemoSay(demo, "Clicked. A button fires on RELEASE, so you can slide off it to change your mind.");
		}
		DemoTooltip(ui, demo, buttonRow, "Press, slide off, release: nothing happens.\nThat is the point.");

		fuiRect repeatRow = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		if(fuiButtonRepeat(ui, repeatRow, "Hold me to repeat")) {
			demo->dragValue += 1.0f;
		}
		DemoTooltip(ui, demo, repeatRow, "Fires on the press, then keeps firing while held.");

		(void)fuiCheckbox(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Show the tile grid", &demo->showGrid);
		(void)fuiCheckbox(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Snap to whole tiles", &demo->snapToGrid);

		fuiBeginGroupBox(ui, "Tool", fuiGroupBoxContentHeight(ui, 3, DEMO_ROW_HEIGHT));
		(void)fuiRadio(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Select", &demo->toolSelection, 0);
		(void)fuiRadio(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Paint", &demo->toolSelection, 1);
		(void)fuiRadio(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "Erase", &demo->toolSelection, 2);
		fuiEndGroupBox(ui);

		char zoomText[32];
		fplStringFormat(zoomText, fplArrayCount(zoomText), "Zoom %.2fx", demo->zoom);
		fuiRect zoomRow = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		(void)fuiSliderFloatEx(ui, zoomRow, "zoom", &demo->zoom, 0.25f, 4.0f, 0.0f, true, true, zoomText, fpl_null, fpl_null);
		DemoTooltip(ui, demo, zoomRow, "Press anywhere on the track to jump the knob there.");

		char gridText[32];
		fplStringFormat(gridText, fplArrayCount(gridText), "Grid %.0f px", (double)demo->gridSize);
		(void)fuiSliderFloatEx(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), "grid", &demo->gridSize, 4.0f, 64.0f, 4.0f, true, demo->snapToGrid, gridText, fpl_null, fpl_null);

		fuiRect dragRow = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		(void)fuiDragFloat(ui, dragRow, "drag", &demo->dragValue, 0.25f, -100.0f, 100.0f);
		DemoTooltip(ui, demo, dragRow, "Drag left and right on the field to change the number.");

		fuiSeparator(ui, fuiLayoutSlot(ui, 12.0f));
		char commandText[96];
		fplStringFormat(commandText, fplArrayCount(commandText), "New ran %d times, Save %d times", demo->newCount, demo->saveCount);
		fuiLabel(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT), commandText);
		(void)fuiCommandButton(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT + 4.0f), &g_demoCommandTable, DEMO_COMMAND_NEW, demo);
		(void)fuiCommandButton(ui, fuiLayoutSlot(ui, DEMO_ROW_HEIGHT + 4.0f), &g_demoCommandTable, DEMO_COMMAND_SAVE, demo);
	}
	fuiEndPanel(ui);
}

static void BuildPickerPanel(fuiContext *ui, DemoState *demo) {
	if(!demo->showPickerPanel) {
		return;
	}
	if(fuiBeginPanelClosable(ui, "Colour", fuiDock_None, 410.0f, 110.0f, 420.0f, 300.0f, &demo->showPickerPanel)) {
		bool didEnd = false;
		fuiRect pickerRect = fuiLayoutRemaining(ui);
		if(fuiColorPicker(ui, pickerRect, "tint", &demo->tint, true, true, fpl_null, &didEnd)) {
			if(didEnd) {
				DemoSay(demo, "The picker reports the END of a drag separately, so one drag is one undoable edit.");
			}
		}
	}
	fuiEndPanel(ui);
}

// ----------------------------------------------------------------------------
// The table
//
// A list view is the one widget where the library and the caller each hold half of the answer: the library
// owns the column widths, the sort and the scroll, and the CALLER owns the rows. So the selection here is
// always a row of the demo's own array and never a position in the list - which is what makes it survive a
// sort without the demo doing anything about it.
// ----------------------------------------------------------------------------

#define DEMO_TABLE_PANEL_WIDTH 560.0f
#define DEMO_TABLE_PANEL_HEIGHT 400.0f
#define DEMO_TABLE_TAB_HEIGHT 28.0f
#define DEMO_TABLE_FOOTER_HEIGHT 26.0f
#define DEMO_ICON_ROW_SCALE 1.4f

static const fuiColumn g_demoTableColumns[DEMO_TABLE_COLUMN_COUNT] = {
	{ "Name", 190.0f },
	{ "Kind", 110.0f },
	{ "Weight", 90.0f },
	{ "Audition", 90.0f },
};

static const char *const g_demoTableTabs[] = { "Entities", "Preview" };

static void BuildTableTab(fuiContext *ui, DemoState *demo, const fuiRect listRect) {
	// Every row's button says Play except the one that is sounding, which says Stop. The labels are per ROW,
	// so one column can read differently on the row that is doing something.
	for(int32_t rowIndex = 0; rowIndex < DEMO_TABLE_ROW_COUNT; ++rowIndex) {
		if(demo->tableRowButtons[rowIndex] == fpl_null) {
			continue; // the row that opted out stays opted out
		}
		demo->tableRowButtons[rowIndex] = (rowIndex == demo->tablePlayingRow) ? "Stop" : "Play";
	}

	fuiListIcons icons = fplZeroInit;
	icons.sheet = demo->iconSheet;
	icons.sheetSize = demo->iconSheetSize;
	icons.columns = (int32_t)DEMO_ICON_CELL_COUNT;
	icons.rows = 1;
	icons.cellForRow = demo->tableIconForRow;
	icons.cellForRowCount = DEMO_TABLE_ROW_COUNT;
	icons.rowScale = DEMO_ICON_ROW_SCALE;

	fuiListRowButtons rowButtons = fplZeroInit;
	rowButtons.column = DEMO_TABLE_COLUMN_COUNT - 1;
	rowButtons.labelForRow = demo->tableRowButtons;

	bool wasActivated = false;
	if(fuiListViewButtons(ui, listRect, "entities", g_demoTableColumns, DEMO_TABLE_COLUMN_COUNT, demo->tableCells, DEMO_TABLE_ROW_COUNT, &demo->tableSelection, &icons, &rowButtons, &wasActivated)) {
		DemoSayFormat(demo, "Picked %s. Sort by a header and it stays picked: the selection is a ROW, not a position.", demo->tableNames[demo->tableSelection]);
	}
	if(wasActivated) {
		DemoSayFormat(demo, "Opened %s, on the second click of a double click.", demo->tableNames[demo->tableSelection]);
	}
	if(rowButtons.clickedRow >= 0) {
		bool wasAlreadyPlaying = (demo->tablePlayingRow == rowButtons.clickedRow);
		demo->tablePlayingRow = wasAlreadyPlaying ? -1 : rowButtons.clickedRow;
		DemoSayFormat(demo, "%s %s - and the row was never picked, because the button owned that press.", wasAlreadyPlaying ? "Stopped" : "Playing", demo->tableNames[rowButtons.clickedRow]);
	}
}

static void BuildPreviewTab(fuiContext *ui, DemoState *demo, const fuiRect contentRect) {
	static const char *const scaleModeNames[] = { "Origin", "Stretch", "Center", "Letterbox" };

	fuiRect controlsRect = fuiRectMake(contentRect.x, contentRect.y, contentRect.w, DEMO_ROW_HEIGHT);
	fuiBeginStackAt(ui, "previewcontrols", fuiAxis_Horizontal, controlsRect, FUI_SPACING_FROM_THEME);
	for(int32_t modeIndex = 0; modeIndex < (int32_t)fplArrayCount(scaleModeNames); ++modeIndex) {
		(void)fuiRadio(ui, fuiLayoutSlot(ui, 110.0f), scaleModeNames[modeIndex], &demo->previewScaleMode, modeIndex);
	}
	fuiEndStack(ui);

	fuiRect flagsRect = fuiRectMake(contentRect.x, contentRect.y + DEMO_ROW_HEIGHT + 4.0f, contentRect.w, DEMO_ROW_HEIGHT);
	fuiBeginStackAt(ui, "previewflags", fuiAxis_Horizontal, flagsRect, FUI_SPACING_FROM_THEME);
	(void)fuiCheckbox(ui, fuiLayoutSlot(ui, 140.0f), "Mirror", &demo->previewIsMirrored);
	(void)fuiCheckbox(ui, fuiLayoutSlot(ui, 190.0f), "Quarter turn", &demo->previewIsTurned);
	fuiEndStack(ui);

	// The whole sheet, drawn into what is left of the panel. The image is purely visual: it hit tests
	// nothing, so the checkboxes above it keep working right up to its edge.
	float boxTop = flagsRect.y + DEMO_ROW_HEIGHT + 8.0f;
	fuiRect box = fuiRectMake(contentRect.x, boxTop, contentRect.w, contentRect.y + contentRect.h - boxTop);
	fuiDrawRectOutline(ui, box, fuiGetTheme(ui)->panelBorderColor, 1.0f);

	fuiImageFlags flags = fuiImageFlags_None;
	if(demo->previewIsMirrored) {
		flags = (fuiImageFlags)(flags | fuiImageFlags_FlipU);
	}
	if(demo->previewIsTurned) {
		flags = (fuiImageFlags)(flags | fuiImageFlags_Rotate90CW);
	}

	fuiImageDesc desc = fplZeroInit;
	desc.texture = demo->iconSheet;
	desc.textureSize = demo->iconSheetSize;
	desc.scaleMode = (fuiImageScaleMode)demo->previewScaleMode;
	desc.flags = flags;
	desc.scaleFactor = 2.0f;
	desc.tint = demo->tint;
	fuiImage(ui, box, &desc);
}

static void BuildTablePanel(fuiContext *ui, DemoState *demo) {
	if(!demo->showTablePanel) {
		return;
	}
	if(fuiBeginPanelClosable(ui, "Entity table", fuiDock_None, 300.0f, 300.0f, DEMO_TABLE_PANEL_WIDTH, DEMO_TABLE_PANEL_HEIGHT, &demo->showTablePanel)) {
		int32_t activeTab = fuiTabControl(ui, fuiLayoutSlot(ui, DEMO_TABLE_TAB_HEIGHT), "tabletabs", g_demoTableTabs, (int32_t)fplArrayCount(g_demoTableTabs));

		// The footer is taken off the bottom BEFORE the rest is handed to the tab, so the tab's content is
		// whatever is left over rather than something that has to know how tall the footer is.
		fuiRect remaining = fuiLayoutRemaining(ui);
		fuiRect footerRect = fuiRectMake(remaining.x, remaining.y + remaining.h - DEMO_TABLE_FOOTER_HEIGHT, remaining.w, DEMO_TABLE_FOOTER_HEIGHT);
		fuiRect contentRect = fuiRectMake(remaining.x, remaining.y, remaining.w, remaining.h - DEMO_TABLE_FOOTER_HEIGHT - 6.0f);

		if(activeTab == 0) {
			BuildTableTab(ui, demo, contentRect);
		} else {
			BuildPreviewTab(ui, demo, contentRect);
		}

		char footerText[DEMO_STATUS_MESSAGE_MAX];
		if(demo->tableSelection >= 0) {
			fplStringFormat(footerText, fplArrayCount(footerText), "Row %d of %d: %s", demo->tableSelection + 1, DEMO_TABLE_ROW_COUNT, demo->tableNames[demo->tableSelection]);
		} else {
			fplCopyString("Click a header to sort, drag a divider to resize a column.", footerText, fplArrayCount(footerText));
		}
		fuiLabel(ui, footerRect, footerText);
	}
	fuiEndPanel(ui);
}

static void BuildListPanel(fuiContext *ui, DemoState *demo) {
	if(!demo->showListPanel) {
		return;
	}
	if(fuiBeginScrollPanelClosable(ui, "Scrolling list", fuiDock_None, 860.0f, 110.0f, 290.0f, 340.0f, &demo->showListPanel)) {
		for(int32_t rowIndex = 0; rowIndex < DEMO_LIST_ROW_COUNT; ++rowIndex) {
			char rowLabel[48];
			fplStringFormat(rowLabel, fplArrayCount(rowLabel), "entity %02d", rowIndex);
			fuiRect rowRect = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
			fuiPushIdInt(ui, rowIndex);
			bool isHovered = false;
			if(fuiCell(ui, rowRect, "row", &isHovered)) {
				demo->selectedRow = rowIndex;
				DemoSay(demo, "A row was chosen. Scroll this panel with the wheel.");
			}
			fuiPopId(ui);
			bool isSelected = (demo->selectedRow == rowIndex);
			if(isSelected || isHovered) {
				fuiColor wash = isSelected ? fuiGetTheme(ui)->menuHighlightColor : fuiGetTheme(ui)->widgetHoveredColor;
				fuiDrawRect(ui, rowRect, wash);
			}
			fuiLabel(ui, rowRect, rowLabel);
		}
	}
	fuiEndPanel(ui);
}

#define DEMO_TREE_PANEL_X 1160.0f
#define DEMO_TREE_PANEL_Y 110.0f
#define DEMO_TREE_PANEL_WIDTH 370.0f
#define DEMO_TREE_PANEL_HEIGHT 620.0f
#define DEMO_TREE_FOOTER_GAP 6.0f
#define DEMO_TREE_ICON_ROW_SCALE 1.4f
#define DEMO_TREE_MENU_ID "treemenu"

/*
	The project explorer.

	Everything the tree reads belongs to the demo - the nodes, the flags saying which folders are open, the
	tables saying which icon a row draws and in what colour. The library writes exactly one of those, the flag
	of a node whose expander was clicked, and copies none of them.
*/
static void BuildTreePanel(fuiContext *ui, DemoState *demo) {
	if(!demo->showTreePanel) {
		return;
	}
	if(fuiBeginPanelClosable(ui, "Project", fuiDock_None, DEMO_TREE_PANEL_X, DEMO_TREE_PANEL_Y, DEMO_TREE_PANEL_WIDTH, DEMO_TREE_PANEL_HEIGHT, &demo->showTreePanel)) {
		const float foldButtonWidth = 168.0f;
		const float guideSwitchWidth = 110.0f;
		const float iconSwitchWidth = 95.0f;
		const float keySwitchWidth = 95.0f;

		// A shut folder and an open one are two different pictures, and which one a row wears is worked out here
		// rather than by the library.
		DemoRefreshTreeIcons(demo);

		fuiRect foldRowRect = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		fuiBeginStackAt(ui, "treefolds", fuiAxis_Horizontal, foldRowRect, FUI_SPACING_FROM_THEME);
		fuiRect expandButtonRect = fuiLayoutSlot(ui, foldButtonWidth);
		bool expandWasClicked = fuiButton(ui, expandButtonRect, "Expand all");
		fuiRect collapseButtonRect = fuiLayoutSlot(ui, foldButtonWidth);
		bool collapseWasClicked = fuiButton(ui, collapseButtonRect, "Collapse all");
		fuiEndStack(ui);

		if(expandWasClicked) {
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, true);
			DemoSay(demo, "All open. Nothing had to be said to the tree: a short one hashes its own flags.");
		}
		if(collapseWasClicked) {
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, false);
			DemoSay(demo, "All shut - and the selection stayed put, because it is a NODE and not a row.");
		}

		fuiRect findButtonRect = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		if(fuiButton(ui, findButtonRect, "Find " DEMO_TREE_BURIED_FILE)) {
			int32_t buriedNode = DemoFindTreeNode(demo, DEMO_TREE_BURIED_FILE);
			// Shut first, so what the two calls underneath do is actually visible rather than already true.
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, false);
			(void)fuiTreeExpandToNode(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, buriedNode);
			demo->treeSelection = buriedNode;
			// Called out here rather than from inside a stack row: a stack pushes an identifier scope, and this
			// has to resolve the tree's identifier in the SAME scope the tree itself is built in.
			fuiTreeReveal(ui, DEMO_TREE_ID, buriedNode);
			DemoSayFormat(demo, "Shut everything, opened the way down to %s and scrolled to it.", DEMO_TREE_BURIED_FILE);
		}

		fuiRect switchRowRect = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		fuiBeginStackAt(ui, "treeswitches", fuiAxis_Horizontal, switchRowRect, FUI_SPACING_FROM_THEME);
		fuiRect guideSwitchRect = fuiLayoutSlot(ui, guideSwitchWidth);
		(void)fuiCheckbox(ui, guideSwitchRect, "Guides", &demo->treeShowsGuides);
		fuiRect iconSwitchRect = fuiLayoutSlot(ui, iconSwitchWidth);
		(void)fuiCheckbox(ui, iconSwitchRect, "Icons", &demo->treeUsesIcons);
		fuiRect keySwitchRect = fuiLayoutSlot(ui, keySwitchWidth);
		(void)fuiCheckbox(ui, keySwitchRect, "Keys", &demo->treeKeyboardIsOn);
		fuiEndStack(ui);

		// The footer is taken off the bottom BEFORE the tree, so the tree is whatever is left rather than
		// something that has to know how tall a footer is.
		fuiRect remaining = fuiLayoutRemaining(ui);
		float footerTop = remaining.y + remaining.h - DEMO_ROW_HEIGHT;
		fuiRect footerRect = fuiRectMake(remaining.x, footerTop, remaining.w, DEMO_ROW_HEIGHT);
		float treeHeight = remaining.h - DEMO_ROW_HEIGHT - DEMO_TREE_FOOTER_GAP;
		fuiRect treeRect = fuiRectMake(remaining.x, remaining.y, remaining.w, treeHeight);

		fuiListIcons icons = fplZeroInit;
		icons.sheet = demo->iconSheet;
		icons.sheetSize = demo->iconSheetSize;
		icons.columns = (int32_t)DEMO_ICON_CELL_COUNT;
		icons.rows = 1;
		icons.cellForRow = demo->treeIconForNode;
		icons.cellForRowCount = demo->treeNodeCount;
		icons.rowScale = DEMO_TREE_ICON_ROW_SCALE;
		icons.tintForRow = demo->treeTintForNode;

		fuiTreeDesc desc = fplZeroInit;
		desc.nodes = demo->treeNodes;
		desc.nodeCount = demo->treeNodeCount;
		desc.isExpanded = demo->treeIsExpanded;
		desc.icons = demo->treeUsesIcons ? &icons : fpl_null;
		desc.showGuides = demo->treeShowsGuides;
		desc.keyboardIsEnabled = demo->treeKeyboardIsOn;

		fuiTreeAction action = fplZeroInit;
		char selectedPath[DEMO_STATUS_MESSAGE_MAX];
		if(fuiTreeViewEx(ui, treeRect, DEMO_TREE_ID, &desc, &demo->treeSelection, &action)) {
			DemoTreePathOf(demo, demo->treeSelection, selectedPath, fplArrayCount(selectedPath));
			DemoSayFormat(demo, "Picked %s", selectedPath);
		}
		if(action.activatedNode >= 0) {
			DemoTreePathOf(demo, action.activatedNode, selectedPath, fplArrayCount(selectedPath));
			DemoSayFormat(demo, "Opened %s, on the second click of a double click.", selectedPath);
		}
		if(action.toggledNode >= 0) {
			const char *toggledLabel = demo->treeNodes[action.toggledNode].label;
			bool isOpen = demo->treeIsExpanded[action.toggledNode];
			DemoSayFormat(demo, "%s %s - the expander owned that click, so nothing was picked.", isOpen ? "Opened" : "Shut", toggledLabel);
		}
		if(action.contextNode >= 0) {
			// Which node the menu is ABOUT is answered by the widget, so the menu and the row under the cursor
			// cannot disagree about what is being acted on.
			demo->treeContextNode = action.contextNode;
			fuiOpenContextMenu(ui, DEMO_TREE_MENU_ID);
		}

		char footerText[DEMO_STATUS_MESSAGE_MAX];
		if(demo->treeSelection >= 0) {
			DemoTreePathOf(demo, demo->treeSelection, footerText, fplArrayCount(footerText));
		} else {
			fplCopyString("Fold with a triangle, pick with a row, walk with the arrow keys.", footerText, fplArrayCount(footerText));
		}
		fuiLabel(ui, footerRect, footerText);
	}
	fuiEndPanel(ui);
}

//! The menu a right click on a tree row opens, acting on THAT row rather than on the selection
static void BuildTreeContextMenu(fuiContext *ui, DemoState *demo) {
	if(fuiBeginContextMenu(ui, DEMO_TREE_MENU_ID)) {
		int32_t contextNode = demo->treeContextNode;
		bool nodeIsInside = (contextNode >= 0) && (contextNode < demo->treeNodeCount);
		int32_t descendantCount = 0;
		bool hasChildren = false;
		bool isOpen = false;
		if(nodeIsInside) {
			descendantCount = demo->treeNodes[contextNode].descendantCount;
			hasChildren = (descendantCount > 0);
			isOpen = hasChildren && demo->treeIsExpanded[contextNode];
		}

		bool canExpand = hasChildren && !isOpen;
		if(fuiMenuItem(ui, "Expand", fpl_null, canExpand)) {
			demo->treeIsExpanded[contextNode] = true;
		}
		if(fuiMenuItem(ui, "Collapse", fpl_null, isOpen)) {
			demo->treeIsExpanded[contextNode] = false;
		}
		if(fuiMenuItem(ui, "Expand subtree", fpl_null, hasChildren)) {
			// The subtree of a node is the run of nodes right behind it, which is what descendantCount says.
			int32_t lastNode = contextNode + descendantCount;
			for(int32_t nodeIndex = contextNode; nodeIndex <= lastNode; ++nodeIndex) {
				demo->treeIsExpanded[nodeIndex] = true;
			}
			DemoSayFormat(demo, "Opened all %d nodes under %s.", descendantCount, demo->treeNodes[contextNode].label);
		}
		fuiMenuSeparator(ui);
		if(fuiMenuItem(ui, "Rename...", fpl_null, nodeIsInside)) {
			fplCopyString(demo->treeNodes[contextNode].label, demo->renameField, fplArrayCount(demo->renameField));
			fuiOpenDialog(ui, "rename");
		}
	}
	fuiEndContextMenu(ui);
}

// ----------------------------------------------------------------------------
// Dialogs
//
// A dialog is opened by IDENTIFIER, from anywhere, and its build is called unconditionally every frame -
// it answers nothing at all while it is closed. So none of this is behind a flag, and a menu row that
// opens one is a single line.
//
// The file browser is the one that shows what the library refuses to do: it reads no directory, so the
// listing below is the caller's and DESCEND is an answer the caller acts on rather than something the
// dialog does by itself.
// ----------------------------------------------------------------------------

#define DEMO_BROWSER_ROOT 0
#define DEMO_BROWSER_LEVELS 1

static const char *const g_demoRootItems[] = { "levels", "readme.txt" };
static const bool g_demoRootIsFolder[] = { true, false };
static const char *const g_demoLevelItems[] = { "..", "gardens-of-ash.lvl", "the-drowned-mill.lvl" };
static const bool g_demoLevelIsFolder[] = { true, false, false };

static void BuildDialogs(fuiContext *ui, DemoState *demo) {
	fuiDialogResult discardResult = fuiMessageBox(ui, "discard", "Discard changes", "The level has unsaved changes. Discard them?", fuiMessageBoxButtons_YesNoCancel);
	if(discardResult == fuiDialogResult_Yes) {
		DemoSay(demo, "Discarded. Enter took the first button, escape the dismissing one.");
	} else if(discardResult == fuiDialogResult_No) {
		DemoSay(demo, "Kept. A message box closes itself as soon as it is answered.");
	} else if(discardResult == fuiDialogResult_Cancel) {
		DemoSay(demo, "Cancelled.");
	}

	fuiDialogResult renameResult = fuiInputBox(ui, "rename", "Rename level", "New name", demo->renameField, (int32_t)fplArrayCount(demo->renameField));
	if(renameResult == fuiDialogResult_Ok) {
		fplCopyString(demo->renameField, demo->nameField, fplArrayCount(demo->nameField));
		DemoSay(demo, "Renamed. The field takes the keyboard as the dialog opens, so you can just type.");
	}

	// The live update switch is the CALLER's: the dialog owns its row and nothing else, which is the only
	// way a colour that drives something real can be judged while it is being dragged.
	fuiDialogResult tintResult = fuiColorDialogEx(ui, "tint", "Pick a colour", &demo->dialogColor, true, &demo->colorLiveUpdate);
	if(tintResult == fuiDialogResult_Ok) {
		demo->tint = demo->dialogColor;
		DemoSay(demo, "Colour taken. The swatch sits over a chequer, so a low alpha reads as transparency.");
	}

	bool isInsideLevels = (demo->browserFolder == DEMO_BROWSER_LEVELS);
	const char *const *browserItems = isInsideLevels ? g_demoLevelItems : g_demoRootItems;
	const bool *browserIsFolder = isInsideLevels ? g_demoLevelIsFolder : g_demoRootIsFolder;
	int32_t browserItemCount = isInsideLevels ? (int32_t)fplArrayCount(g_demoLevelItems) : (int32_t)fplArrayCount(g_demoRootItems);
	const char *locationLabel = isInsideLevels ? "/levels" : "/";
	int32_t browserOutIndex = -1;
	fuiFileBrowserResult browserResult = fuiFileBrowser(ui, "browse", "Save level as", locationLabel, browserItems, browserIsFolder, browserItemCount, &demo->browserSelection, fpl_null, demo->browserName, (int32_t)fplArrayCount(demo->browserName), &browserOutIndex);
	if(browserResult == fuiFileBrowserResult_Descend) {
		// The dialog stays open and the caller lists the folder instead. Which folder is answered from the
		// ROW that was activated, not from where we happen to be: row zero of the levels listing is the ".."
		// that goes back up. Resetting the selection matters too, or the new listing opens with whatever row
		// index the old one was sitting on already highlighted.
		bool wentUpADirectory = isInsideLevels && (browserOutIndex == 0);
		demo->browserFolder = wentUpADirectory ? DEMO_BROWSER_ROOT : DEMO_BROWSER_LEVELS;
		demo->browserSelection = -1;
	} else if(browserResult == fuiFileBrowserResult_Accept) {
		// A SAVING browser stays open on accept, on purpose: this is where an overwrite prompt goes, and it
		// opens ON TOP of the browser rather than instead of it. Escape then closes one level per press.
		bool nameIsTaken = false;
		for(int32_t itemIndex = 0; itemIndex < browserItemCount; ++itemIndex) {
			if(!browserIsFolder[itemIndex] && fplIsStringEqual(browserItems[itemIndex], demo->browserName)) {
				nameIsTaken = true;
			}
		}
		if(nameIsTaken) {
			fuiOpenDialog(ui, "overwrite");
		} else {
			fuiCloseDialog(ui, "browse");
			DemoSay(demo, "Saved under a new name. Nothing was written, but the dialog really did answer.");
		}
	}

	fuiDialogResult overwriteResult = fuiMessageBox(ui, "overwrite", "Overwrite", "That file already exists. Overwrite it?", fuiMessageBoxButtons_YesNo);
	if(overwriteResult == fuiDialogResult_Yes) {
		fuiCloseDialog(ui, "browse");
		DemoSay(demo, "Overwritten. That prompt was a modal on top of a modal.");
	} else if(overwriteResult == fuiDialogResult_No) {
		DemoSay(demo, "Kept. The browser underneath is still open - escape closes one level at a time.");
	}
}

static void BuildStatusBar(fuiContext *ui, DemoState *demo, const fuiRect statusRect) {
	fuiBeginStatusBar(ui, "statusbar", statusRect);
	fuiStatusText(ui, demo->statusMessage);

	char rightText[64];
	fplStringFormat(rightText, fplArrayCount(rightText), "%.0f fps", (double)demo->framesPerSecond);
	fuiStatusTextRight(ui, rightText);

	fuiVec2 mousePosition = fuiGetMousePosition(ui);
	char mouseText[64];
	fplStringFormat(mouseText, fplArrayCount(mouseText), "%.0f, %.0f", (double)mousePosition.x, (double)mousePosition.y);
	fuiStatusTextRight(ui, mouseText);

	fuiStatusTextRight(ui, demo->uiOwnedTheMouseLastFrame ? "UI has the mouse" : "world has the mouse");

	// Which of the two demos this is, said out loud. The right hand items are laid out from the right edge
	// inwards, so this one ends up furthest left of them.
	fuiStatusTextRight(ui, DEMO_HOST_LABEL);
	fuiEndStatusBar(ui);
}

static void BuildUserInterface(fuiContext *ui, DemoState *demo, const bool rightWasPressed) {
	// The three bars are cut off the root container BEFORE anything else, so their width comes from whatever
	// encloses them and their thickness from the theme. Nothing here names a pixel: a menu bar is one menu
	// row tall, a tool strip one strip button, a status bar one line of text - and a restyled theme moves all
	// three without a constant to chase.
	fuiRect menuBarRect = fuiLayoutDock(ui, fuiDock_Top, fuiMenuBarHeight(ui));
	fuiRect toolStripRect = fuiLayoutDock(ui, fuiDock_Top, fuiToolStripThickness(ui));
	fuiRect statusBarRect = fuiLayoutDock(ui, fuiDock_Bottom, fuiStatusBarHeight(ui));

	// The shortcuts first, so a Ctrl+Q is answered even while the pointer is somewhere harmless. Nothing
	// is dispatched while a text field has the keyboard, which is why typing an S into the name field
	// does not save.
	(void)fuiDispatchShortcuts(ui, &g_demoCommandTable, demo);

	// A right press the interface did not want is the canvas's, and opens the context menu there. Asked of
	// the PREVIOUS frame, because nothing has been built yet and fuiWantsMouse would answer "no" for a
	// cursor sitting squarely on a panel. See DemoState.uiOwnedTheMouseLastFrame.
	if(rightWasPressed && !demo->uiOwnedTheMouseLastFrame) {
		fuiOpenContextMenu(ui, "canvasmenu");
	}

	BuildToolStrip(ui, demo, toolStripRect);

	BuildWidgetsPanel(ui, demo);
	BuildPickerPanel(ui, demo);
	BuildListPanel(ui, demo);
	BuildTablePanel(ui, demo);
	BuildTreePanel(ui, demo);

	BuildStatusBar(ui, demo, statusBarRect);

	// Popups float ABOVE the docked layout but are BUILT last, which is what lets a later call take the
	// cursor from an earlier one with no z ordering anywhere.
	BuildMenuBar(ui, demo, menuBarRect);

	BuildTreeContextMenu(ui, demo);

	if(fuiBeginContextMenu(ui, "canvasmenu")) {
		if(fuiMenuItem(ui, "Bring the panels back", fpl_null, true)) {
			demo->showWidgetsPanel = true;
			demo->showPickerPanel = true;
			demo->showListPanel = true;
			demo->showTablePanel = true;
			demo->showTreePanel = true;
			DemoSay(demo, "All five panels are open again.");
		}
		if(fuiMenuItemCheck(ui, "Hover tooltips", demo->showTooltips, true)) {
			demo->showTooltips = !demo->showTooltips;
		}
		fuiMenuSeparator(ui);
		(void)fuiMenuItemCommand(ui, &g_demoCommandTable, DEMO_COMMAND_QUIT, demo);
	}
	fuiEndContextMenu(ui);

	// Built LAST, because a dialog's backdrop covers everything - including a menu popup that was open when
	// the row opening the dialog was clicked.
	BuildDialogs(ui, demo);
}

// ----------------------------------------------------------------------------
// The framework side
// ----------------------------------------------------------------------------

//! Expands one channel of coverage into the white-with-coverage-in-alpha texels the sprite path modulates
static void DemoExpandIconSheet(const unsigned char *coveragePixels, unsigned char *outTexels) {
	const unsigned char opaqueWhite = 255;
	for(size_t texelIndex = 0; texelIndex < (size_t)DEMO_ICON_TEXEL_COUNT; ++texelIndex) {
		size_t byteAt = texelIndex * (size_t)DEMO_ICON_TEXEL_CHANNELS;
		outTexels[byteAt + 0] = opaqueWhite;
		outTexels[byteAt + 1] = opaqueWhite;
		outTexels[byteAt + 2] = opaqueWhite;
		outTexels[byteAt + 3] = coveragePixels[texelIndex];
	}
}

//! Loads the interface font out of final_fonts.h and queues its atlas for upload
static bool LoadDemoFont(RenderState *renderState, FontAsset *outFontAsset) {
	// TODO: proper memory allocator
	MemoryAllocator *allocator = fpl_null;

	const uint32_t firstFontInTheFile = 0;
	// On, so the demo exercises the adapter's kerning bridge as well as its glyphs. Without it the interface
	// lays text out on the plain advances, which is what it does on a font that carries no kerning at all.
	const bool loadKerning = true;
	if(!FontLoadFromMemory(allocator, ptr_fontBitstreamVeraRegular, sizeOf_fontBitstreamVeraRegular, firstFontInTheFile, DEMO_FONT_PIXEL_HEIGHT, DEMO_FONT_FIRST_CODEPOINT, DEMO_FONT_LAST_CODEPOINT, DEMO_FONT_ATLAS_SIDE, DEMO_FONT_ATLAS_SIDE, loadKerning, &outFontAsset->desc)) {
		return(false);
	}

	const LoadedFont *loadedFont = &outFontAsset->desc;
	const uint32_t coverageBytesPerTexel = 1;
	const bool isTopDown = false;
	const bool isPreMultiplied = false;
	RenderPushTexture(renderState, &outFontAsset->texture, loadedFont->atlasAlphaBitmap, loadedFont->atlasWidth, loadedFont->atlasHeight, coverageBytesPerTexel, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, isTopDown, isPreMultiplied);
	return(true);
}

/*
	Points the interface at the two textures once they exist.

	RenderPushTexture only QUEUES an upload: the handle behind it is filled in when the pipeline drains the
	queue at the end of the frame. So at GameInit time the font's atlas texture is still null, and a fuiFont
	built there would draw no text at all. Called at the top of every frame and does nothing after the first
	one that finds real handles.
*/
static void WireQueuedTextures(GameState *state) {
	if(state->areTexturesWired || state->uiFont.texture == fpl_null) {
		return;
	}

	// Refreshed IN PLACE, because the address of this font is what fuiInit was given.
	if(!fuiFontFromFontAsset(&state->uiFont, &state->font)) {
		return;
	}
	fuiSetFont(&state->ui, &state->font);

	state->demo.iconSheet = (fuiTextureId)(uintptr_t)state->iconTexture;
	state->demo.iconSheetSize = fuiV2((float)DEMO_ICON_SHEET_WIDTH, (float)DEMO_ICON_SHEET_HEIGHT);
	state->areTexturesWired = true;
}

// ----------------------------------------------------------------------------
// The world behind the interface
// ----------------------------------------------------------------------------

//! A grid, so there is something under the panels to see them float over -- and something fuiWantsMouse can
//! protect. Pushed as render commands in screen pixels, y-up, before the interface goes on top.
static void RenderBackdrop(RenderState *renderState, const DemoState *demo, const int32_t windowWidth, const int32_t windowHeight) {
	// Mixed toward the accent, so the world behind the panels reads as this demo's too and not only its chrome.
	const Vec4f backdropColor = V4fInit(0.05f, 0.10f, 0.11f, 1.0f);
	const float gridAlpha = 0.10f;
	const float minimumGridSpacing = 4.0f;
	const float gridLineWidth = 1.0f;

	// The full window, not an aspect corrected viewport: an interface is measured in pixels and every one of
	// them belongs to it.
	RenderPushViewport(renderState, 0, 0, windowWidth, windowHeight);
	RenderPushClear(renderState, backdropColor, ClearFlags_Color | ClearFlags_Depth);
	if(!demo->showGrid) {
		return;
	}

	Mat4f screenProjection = M4fOrthoRH(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, 0.0f, 1.0f);
	RenderSetMatrix(renderState, &screenProjection);

	float spacing = demo->gridSize * demo->zoom;
	if(spacing < minimumGridSpacing) {
		spacing = minimumGridSpacing;
	}
	Vec4f gridColor = V4fInit(demo->tint.r, demo->tint.g, demo->tint.b, gridAlpha);
	for(float x = 0.0f; x < (float)windowWidth; x += spacing) {
		Vec2f top = V2fInit(x, 0.0f);
		Vec2f bottom = V2fInit(x, (float)windowHeight);
		RenderPushLine(renderState, top, bottom, gridColor, gridLineWidth);
	}
	for(float y = 0.0f; y < (float)windowHeight; y += spacing) {
		Vec2f left = V2fInit(0.0f, y);
		Vec2f right = V2fInit((float)windowWidth, y);
		RenderPushLine(renderState, left, right, gridColor, gridLineWidth);
	}
}

// ----------------------------------------------------------------------------
// The game platform's entry points
// ----------------------------------------------------------------------------

extern bool GameInit(GameMemory *gameMemory, const int argumentCount, char **arguments) {
	(void)argumentCount;
	(void)arguments;
	if(gameMemory == fpl_null) {
		return(false);
	}

	GameState *state = fmemPushStruct(gameMemory->memory, GameState, fmemPushFlags_Clear);
	if(state == fpl_null) {
		LogWrite(LogLevel_Fatal, DEMO_LOG_CATEGORY, "Insufficient game memory for the demo state (%zu bytes)", sizeof(GameState));
		return(false);
	}
	gameMemory->game = state;

	// The framework's GameConfiguration says nothing about window size, so the demo asks for one wide enough
	// to stand an explorer column beside the panels that were already there.
	fplSetWindowSize(DEMO_WINDOW_WIDTH, DEMO_WINDOW_HEIGHT);

	RenderState *renderState = gameMemory->render;

	if(!LoadDemoFont(renderState, &state->uiFont)) {
		LogWrite(LogLevel_Fatal, DEMO_LOG_CATEGORY, "Failed to load the interface font");
		return(false);
	}

	// Built with the atlas texture still null - see WireQueuedTextures. Everything else about the font,
	// including its metrics and every glyph, is final from here on.
	if(!fuiFontFromFontAsset(&state->uiFont, &state->font)) {
		LogWrite(LogLevel_Fatal, DEMO_LOG_CATEGORY, "The loaded font carries no glyphs");
		return(false);
	}

	// The interface allocates out of the GAME memory block, so the demo keeps the framework's promise of
	// doing no malloc of its own.
	fuiAllocator allocator;
	fuiAllocatorFromMemoryBlock(gameMemory->memory, &allocator);
	if(!fuiInit(&state->ui, &state->font, &allocator)) {
		LogWrite(LogLevel_Fatal, DEMO_LOG_CATEGORY, "Failed to initialize the user interface");
		return(false);
	}

	// Optional, and worth wiring: without it the text fields simply have no clipboard.
	fuiPlatform platform;
	fuiPlatformFromFPL(&platform);
	fuiSetPlatform(&state->ui, &platform);

	// The one deliberate deviation from FUI_Test's look. fuiGetTheme hands back the LIVE theme, so a single
	// field is all it takes to restain every highlight the interface draws.
	fuiTheme *theme = fuiGetTheme(&state->ui);
	theme->accentColor = g_demoAccentColor;

	DemoInit(&state->demo);

	// The only asset in the demo that is not a font: eight icon cells the demo draws itself. The coverage is
	// scratch, the expanded texels are not - the upload reads them after this function has returned.
	unsigned char coveragePixels[DEMO_ICON_TEXEL_COUNT];
	DemoDrawIconSheet(coveragePixels);
	DemoExpandIconSheet(coveragePixels, state->iconTexels);
	const bool isTopDown = false;
	const bool isPreMultiplied = false;
	RenderPushTexture(renderState, &state->iconTexture, state->iconTexels, DEMO_ICON_SHEET_WIDTH, DEMO_ICON_SHEET_HEIGHT, DEMO_ICON_TEXEL_CHANNELS, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, isTopDown, isPreMultiplied);

	LogWrite(LogLevel_Info, DEMO_LOG_CATEGORY, "%s is up", DEMO_WINDOW_TITLE);
	return(true);
}

extern void GameRelease(GameMemory *gameMemory) {
	if(gameMemory == fpl_null || gameMemory->game == fpl_null) {
		return;
	}

	GameState *state = gameMemory->game;
	fuiRelease(&state->ui);

	// TODO: proper memory allocator
	MemoryAllocator *allocator = fpl_null;
	FontAssetFree(allocator, &state->uiFont);
}

extern bool IsGameExiting(GameMemory *gameMemory) {
	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);
	// The Quit command clears the demo's own flag, from a menu row, a strip button or Ctrl+Q.
	return(!state->demo.isRunning);
}

extern void GameInput(GameMemory *gameMemory, const Input *input) {
	// Nothing to do. An immediate mode interface reads the input WHILE it is built, and that happens in
	// GameRender - so there is no earlier pass to feed here.
	(void)gameMemory;
	(void)input;
}

extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	// Nothing to do either. The demo has no simulation: every value on screen is edited by the widget that
	// draws it, which is the whole idea of an immediate mode interface.
	(void)gameMemory;
	(void)input;
}

extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha) {
	(void)alpha;

	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);

	RenderState *renderState = gameMemory->render;
	fplAssert(renderState != fpl_null);

	WireQueuedTextures(state);

	state->demo.framesPerSecond = input->framesPerSeconds;

	RenderBackdrop(renderState, &state->demo, input->windowSize.x, input->windowSize.y);

	// The whole bridge, in one call. Everything the library is told about this frame comes out of the
	// platform's own input snapshot.
	fuiInput uiInput;
	fuiInputFromFinalGame(input, &uiInput);

	// A right press the interface did not want is the canvas's. Read off the DOWN edge, which the framework
	// keeps even for a tap that fell between two keyboard polls.
	bool rightWasPressed = ButtonWentDown(input->mouse.right);

	// One pass: build the whole interface between begin and end, and let the library work out what was
	// clicked. That is the entire contract, and it is identical to FUI_Test's.
	fuiBeginFrame(&state->ui, &uiInput, fuiPass_Both);
	BuildUserInterface(&state->ui, &state->demo, rightWasPressed);
	fuiEndFrame(&state->ui);

	// Asked here and nowhere else: this is the first moment the answer is complete for this frame.
	state->demo.uiOwnedTheMouseLastFrame = fuiWantsMouse(&state->ui);

	const fuiDrawData *drawData = fuiGetDrawData(&state->ui);
	fuiRenderDrawData(renderState, drawData, &state->uiFont);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char **argv) {
	GameConfiguration config = fplZeroInit;
	config.title = DEMO_WINDOW_TITLE;
	int result = GameMain(&config, argc, argv);
	return(result);
}
