/*
Name:
	FUI_Test

Description:
	An interactive demo for final_ui.h, on FPL and legacy OpenGL.

	Everything the library has: a menu bar with a submenu, a context menu, a command table whose
	shortcuts fire from the keyboard AND print themselves into their own menu rows, tool strips, floating
	panels you can drag, resize and collapse, one of every widget, a colour picker, tooltips, and a status
	bar. The font is real: fui_font_stbtt.h bakes one of final_fonts.h's embedded faces with stb_truetype,
	so there is no asset to ship and nothing to find at runtime.

	This is also the smallest honest answer to "what does it take to use this library". Four things:

	  1. Bake a font and upload its atlas                (fuiStbttFontBake + fuiGL1UploadFontAtlas)
	  2. Fill a fuiInput each frame                      (BuildInput, below - about sixty lines)
	  3. Build the interface between begin and end       (BuildUserInterface, below)
	  4. Drain fuiGetDrawData through a backend          (fuiGL1Render)

	final_ui.h itself pulls in nothing but the C standard library. FPL, stb_truetype and OpenGL all appear
	in THIS file and in the two headers next to it, never in the library.

	The loop is ONE pass (fuiPass_Both), which is what a plain demo wants: build the interface once per
	frame and let hover resolve against the previous frame's layout. A host that has to know whether the
	interface wants the mouse before it decides what to do with the cursor - a game with a world behind
	the interface, say - builds twice instead, fuiPass_Interact then fuiPass_Draw.

Requirements:
	- C99 compiler
	- OpenGL 1.1 (fixed function, which is all the backend here uses)

Build (from the repository root):
	gcc -std=c99 demos/FUI_Test/fui_test.c -I . -I demos/additions -I demos/dependencies -o fui_test -lm -ldl
	./fui_test

	Or with cmake:  cmake -S demos/FUI_Test -B build/fui_test && cmake --build build/fui_test

License:
	MIT License, Copyright (c) 2017-2026 Torsten Spaete
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

// stb_truetype's implementation, and the embedded TrueType faces it bakes from.
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include <final_fonts.h>

#define FUI_IMPLEMENTATION
#include <final_ui.h>

#define FUI_STBTT_IMPLEMENTATION
#include <fui_font_stbtt.h>

#define FUI_GL1_IMPLEMENTATION
#include <fui_backend_gl1.h>

#define FUI_INPUT_FPL_IMPLEMENTATION
#include <fui_input_fpl.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define DEMO_WINDOW_TITLE "final_ui.h demo (FPL + OpenGL)"

// Named in the status bar, because FUI_Framework builds this same interface on the Final Framework and a
// screenshot of either one has to say which is which. This demo keeps the theme's default amber accent; the
// other one restains it.
#define DEMO_HOST_LABEL "FPL + OpenGL 1.1"
#define DEMO_WINDOW_WIDTH 1560
#define DEMO_WINDOW_HEIGHT 800

// Baked once, above the largest text on screen, so every size drawn is a reduction of the atlas.
#define DEMO_FONT_PIXEL_HEIGHT 34.0f
#define DEMO_FONT_ATLAS_SIDE 512u

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

// The icon sheets the demo draws itself: four square cells in a row, no asset to ship, and a worked example
// of what fuiListIcons wants. There are TWO of them, of the same four shapes, because a sheet can reach the
// backend by either of two roads and the demo runs both - one channel of COVERAGE, uploaded exactly the way
// the font atlas is and stained one shade by the vertex tint, and four channels of COLOR the sheet carries
// itself. The Entity table switches between them.
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
#define DEMO_ICON_SHEET_CHANNELS 4u
#define DEMO_ICON_SHEET_COLOR_BYTES (DEMO_ICON_SHEET_WIDTH * DEMO_ICON_SHEET_HEIGHT * DEMO_ICON_SHEET_CHANNELS)

#define DEMO_TREE_NODE_MAX 64
#define DEMO_TREE_ID "projecttree"
//! The file the Find button digs out, named rather than indexed so the tree table can be edited freely
#define DEMO_TREE_BURIED_FILE "water-01.wav"

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
	//! One tint per row, all of them left FULLY TRANSPARENT except the row that is sounding
	fuiColor tableIconTintForRow[DEMO_TABLE_ROW_COUNT];
	int32_t tableSelection;
	//! Which row is sounding, so its button reads Stop while every other one reads Play
	int32_t tablePlayingRow;
	//! The coverage sheet, drawn into an alpha bitmap at startup and uploaded like the font atlas
	fuiTextureId iconSheet;
	fuiVec2 iconSheetSize;
	//! The same shapes in four channels, uploaded through fuiGL1UploadImageRGBA
	fuiTextureId iconSheetColor;
	fuiVec2 iconSheetColorSize;

	// What the Entity table does with its icons, so every field of fuiListIcons that changes the picture can
	// be switched while looking at it.
	bool tableUsesColorSheet;
	bool tableShowsHeaderIcons;
	bool tableIconsOnKindColumn;
	bool tableIconsOnly;
	//! Which image the preview tab is showing, and how
	int32_t previewScaleMode;
	bool previewIsMirrored;
	bool previewIsTurned;

	// The project tree. The nodes and their flags are the DEMO's, exactly like the table's rows: the library
	// reads them, writes a flag when an expander is clicked, and copies nothing.
	fuiTreeNode treeNodes[DEMO_TREE_NODE_MAX];
	bool treeIsExpanded[DEMO_TREE_NODE_MAX];
	int32_t treeIconForNode[DEMO_TREE_NODE_MAX];
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
	so what this panel shows is the same on every machine it is run on. What a real explorer would swap out is
	this table and nothing else.
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

//! Whether one texel of a cell lies inside that cell's shape. BOTH sheets are drawn from this, so the
//! coverage one and the color one are the same four pictures and only differ in what they are painted with.
static bool DemoIconTexelIsInk(const int32_t cellIndex, const int32_t x, const int32_t y) {
	const int32_t cellSide = (int32_t)DEMO_ICON_CELL_SIDE;
	const float centre = (float)cellSide * 0.5f;
	const float outerRadius = (float)cellSide * 0.36f;
	const float innerRadius = (float)cellSide * 0.24f;
	// Compared as squares, so the sheet needs no square root and the demo no math header of its own.
	const float outerRadiusSquared = outerRadius * outerRadius;
	const float innerRadiusSquared = innerRadius * innerRadius;
	const int32_t squareInset = cellSide / 5;

	float offsetX = ((float)x + 0.5f) - centre;
	float offsetY = ((float)y + 0.5f) - centre;
	float distanceSquared = offsetX * offsetX + offsetY * offsetY;
	float distanceFromCentreX = (offsetX < 0.0f) ? -offsetX : offsetX;
	float distanceFromCentreY = (offsetY < 0.0f) ? -offsetY : offsetY;

	// The four shapes the project tree is drawn with, measured off the cell so they scale with it. A folder is a
	// tab over a body, and the open one is the same folder with its lid lifted clear of it.
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

	bool isInk = false;
	switch(cellIndex) {
		case 0: // a solid block
			isInk = (x >= squareInset) && (x < cellSide - squareInset) && (y >= squareInset) && (y < cellSide - squareInset);
			break;
		case 1: // a triangle standing on its base
			isInk = (y >= squareInset) && (y < cellSide - squareInset) && (distanceFromCentreX <= (float)(y - squareInset) * 0.5f);
			break;
		case 2: // a ring
			isInk = (distanceSquared <= outerRadiusSquared) && (distanceSquared >= innerRadiusSquared);
			break;
		case 3: // a diamond
			isInk = (distanceFromCentreX + distanceFromCentreY) <= outerRadius;
			break;
		case DEMO_ICON_CELL_FOLDER_SHUT:
		{
			bool isTab = (x >= folderLeft) && (x < folderTabRight) && (y >= folderTabTop) && (y < folderTabBottom);
			bool isBody = (x >= folderLeft) && (x < folderRight) && (y >= folderTabBottom) && (y < folderBottom);
			isInk = isTab || isBody;
		} break;
		case DEMO_ICON_CELL_FOLDER_OPEN:
		{
			// The same tab, a lid where the body's top edge was, and the body itself dropped and pushed right -
			// which is a folder with its lid taken off it rather than a differently shaped folder.
			bool isTab = (x >= folderLeft) && (x < folderTabRight) && (y >= folderTabTop) && (y < folderTabBottom);
			bool isLid = (x >= folderLeft) && (x < folderRight) && (y >= folderTabBottom) && (y < folderLidBottom);
			bool isBody = (x >= folderOpenBodyLeft) && (x < folderRight) && (y >= folderOpenBodyTop) && (y < folderBottom);
			isInk = isTab || isLid || isBody;
		} break;
		case DEMO_ICON_CELL_FILE:
		{
			bool isSheet = (x >= fileLeft) && (x < fileRight) && (y >= fileTop) && (y < fileBottom);
			int32_t foldLeft = fileRight - fileFoldSize;
			int32_t foldBottom = fileTop + fileFoldSize;
			bool isCornerFold = (x >= foldLeft) && (y < foldBottom) && (((x - foldLeft) + (foldBottom - y)) > fileFoldSize);
			isInk = isSheet && !isCornerFold;
		} break;
		default: // the same sheet with lines written on it, which is what a level file gets
		{
			bool isSheet = (x >= fileLeft) && (x < fileRight) && (y >= fileTop) && (y < fileBottom);
			int32_t foldLeft = fileRight - fileFoldSize;
			int32_t foldBottom = fileTop + fileFoldSize;
			bool isCornerFold = (x >= foldLeft) && (y < foldBottom) && (((x - foldLeft) + (foldBottom - y)) > fileFoldSize);
			int32_t lineLeft = fileLeft + cellSide / 10;
			int32_t lineRight = fileRight - cellSide / 10;
			int32_t firstLineTop = fileTop + fileFoldSize + cellSide / 12;
			int32_t lineSpacing = cellSide / 7;
			bool isBetweenTheLines = false;
			for(int32_t lineIndex = 0; lineIndex < 3; ++lineIndex) {
				int32_t lineTop = firstLineTop + lineIndex * lineSpacing;
				bool isOnThisLine = (y >= lineTop) && (y < lineTop + cellSide / 16 + 1) && (x >= lineLeft) && (x < lineRight);
				if(isOnThisLine) {
					isBetweenTheLines = true;
				}
			}
			isInk = isSheet && !isCornerFold && !isBetweenTheLines;
		} break;
	}
	return(isInk);
}

/*
	The COVERAGE sheet - a square, a triangle, a ring and a diamond, one per kind, in a single channel. It goes
	up through the same call the font atlas does, because to this backend an atlas IS a coverage bitmap, and a
	list icon and a glyph ask exactly the same thing of it.

	The library never learns what a cell MEANS. It draws cell N for the row whose entry says N, and which cell
	a row gets is the table further down.
*/
static void DemoDrawIconSheet(unsigned char *coveragePixels) {
	const int32_t cellSide = (int32_t)DEMO_ICON_CELL_SIDE;
	const int32_t sheetWidth = (int32_t)DEMO_ICON_SHEET_WIDTH;

	memset(coveragePixels, 0, (size_t)sheetWidth * (size_t)cellSide);
	for(int32_t cellIndex = 0; cellIndex < (int32_t)DEMO_ICON_CELL_COUNT; ++cellIndex) {
		for(int32_t y = 0; y < cellSide; ++y) {
			for(int32_t x = 0; x < cellSide; ++x) {
				bool isInk = DemoIconTexelIsInk(cellIndex, x, y);
				if(!isInk) {
					continue;
				}
				int32_t sheetX = cellIndex * cellSide + x;
				coveragePixels[(size_t)y * (size_t)sheetWidth + (size_t)sheetX] = 255;
			}
		}
	}
}

// What each kind is painted in on the COLOR sheet, one entry per cell.
static const fuiColor g_demoIconKindColors[DEMO_ICON_CELL_COUNT] = {
	{ 0.60f, 0.64f, 0.72f, 1.0f }, // Solid, a cool grey
	{ 0.40f, 0.76f, 0.44f, 1.0f }, // Prop, green
	{ 0.94f, 0.62f, 0.26f, 1.0f }, // Trigger, amber
	{ 0.96f, 0.86f, 0.36f, 1.0f }, // Light, a pale yellow
	{ 0.92f, 0.74f, 0.36f, 1.0f }, // Folder shut, a manila amber
	{ 0.96f, 0.82f, 0.50f, 1.0f }, // Folder open, the same amber lifted
	{ 0.76f, 0.81f, 0.88f, 1.0f }, // File, paper
	{ 0.62f, 0.80f, 0.90f, 1.0f }, // Level file, a cooler paper
};

/*
	The same four shapes in FOUR channels, each kind in its own color and shading from a light top edge to the
	full color at the bottom. That gradient is the whole point of the sheet: a coverage sheet is stained ONE
	shade by the vertex tint, and no tint can put two colors inside a single glyph.

	Straight alpha and not premultiplied, which is what fuiGL1UploadImageRGBA wants and what the stb_image
	loaders in final_assets.h hand over. Everything outside a shape is left transparent AND black, so a linear
	filter fading off the edge of a shape fades toward nothing rather than toward a dark fringe.
*/
static void DemoDrawColorIconSheet(unsigned char *rgbaPixels) {
	const int32_t cellSide = (int32_t)DEMO_ICON_CELL_SIDE;
	const int32_t sheetWidth = (int32_t)DEMO_ICON_SHEET_WIDTH;
	const int32_t bytesPerTexel = (int32_t)DEMO_ICON_SHEET_CHANNELS;
	// How far the top row of a cell is lifted toward white. Enough to read as lit from above at 44 pixels.
	const float topHighlightStrength = 0.55f;
	const float fullChannel = 255.0f;

	memset(rgbaPixels, 0, (size_t)DEMO_ICON_SHEET_COLOR_BYTES);
	for(int32_t cellIndex = 0; cellIndex < (int32_t)DEMO_ICON_CELL_COUNT; ++cellIndex) {
		fuiColor kindColor = g_demoIconKindColors[cellIndex];
		for(int32_t y = 0; y < cellSide; ++y) {
			float distanceDownTheCell = (float)y / (float)(cellSide - 1);
			float highlight = topHighlightStrength * (1.0f - distanceDownTheCell);
			float red = kindColor.r + (1.0f - kindColor.r) * highlight;
			float green = kindColor.g + (1.0f - kindColor.g) * highlight;
			float blue = kindColor.b + (1.0f - kindColor.b) * highlight;
			for(int32_t x = 0; x < cellSide; ++x) {
				bool isInk = DemoIconTexelIsInk(cellIndex, x, y);
				if(!isInk) {
					continue;
				}
				int32_t sheetX = cellIndex * cellSide + x;
				size_t texelAt = ((size_t)y * (size_t)sheetWidth + (size_t)sheetX) * (size_t)bytesPerTexel;
				rgbaPixels[texelAt + 0] = (unsigned char)(red * fullChannel);
				rgbaPixels[texelAt + 1] = (unsigned char)(green * fullChannel);
				rgbaPixels[texelAt + 2] = (unsigned char)(blue * fullChannel);
				rgbaPixels[texelAt + 3] = (unsigned char)fullChannel;
			}
		}
	}
}

//! Every node's icon, worked out from what it IS and from whether it happens to be folded open right now
/*
	Called every frame before the tree is built, which is cheap for a table this size and always right. It is
	also the whole point of the icon table belonging to the caller: the library never learns that cell four is a
	shut folder and cell five an open one, it just draws the cell the row asks for.
*/
static void DemoRefreshTreeIcons(DemoState *demo) {
	for(int32_t nodeIndex = 0; nodeIndex < demo->treeNodeCount; ++nodeIndex) {
		int32_t leafIconCell = g_demoTreeEntries[nodeIndex].leafIconCell;
		bool isFolder = (leafIconCell == DEMO_TREE_FOLDER);
		if(!isFolder) {
			demo->treeIconForNode[nodeIndex] = leafIconCell;
			continue;
		}
		bool isOpen = demo->treeIsExpanded[nodeIndex];
		demo->treeIconForNode[nodeIndex] = isOpen ? DEMO_ICON_CELL_FOLDER_OPEN : DEMO_ICON_CELL_FOLDER_SHUT;
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
// FPL -> final_ui.h
//
// The whole bridge - the key table, the polled device state, the drained events and the clipboard -
// lives in fui_input_fpl.h next door, because every FPL program that draws this interface needs the
// same sixty lines and none of them should carry their own copy of the key table.
// ----------------------------------------------------------------------------

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
		if(fuiMenuItem(ui, "Dialog with an icon", fpl_null, true)) {
			fuiOpenDialog(ui, "about");
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
			DemoSay(demo, "final_ui.h: one header, no dependencies. This window is FPL, OpenGL 1.1 and about 500 lines.");
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

// One cell per column HEADER, read out of the same sheet the rows are. The button column asks for none, which
// is what a negative entry says.
static const int32_t g_demoTableHeaderIcons[DEMO_TABLE_COLUMN_COUNT] = { 0, 1, 2, -1 };

#define DEMO_TABLE_CONTROLS_GAP 6.0f

static void BuildTableTab(fuiContext *ui, DemoState *demo, const fuiRect contentRect) {
	// A row of switches over the list, one per thing fuiListIcons can be told to do, so each of them can be
	// turned on and off with the list it changes in sight.
	const float sheetSwitchWidth = 115.0f;
	const float headerSwitchWidth = 130.0f;
	const float columnSwitchWidth = 100.0f;
	const float textSwitchWidth = 115.0f;

	fuiRect controlsRect = fuiRectMake(contentRect.x, contentRect.y, contentRect.w, DEMO_ROW_HEIGHT);
	fuiBeginStackAt(ui, "tableiconswitches", fuiAxis_Horizontal, controlsRect, FUI_SPACING_FROM_THEME);
	fuiRect sheetSwitchRect = fuiLayoutSlot(ui, sheetSwitchWidth);
	(void)fuiCheckbox(ui, sheetSwitchRect, "RGBA sheet", &demo->tableUsesColorSheet);
	fuiRect headerSwitchRect = fuiLayoutSlot(ui, headerSwitchWidth);
	(void)fuiCheckbox(ui, headerSwitchRect, "Header icons", &demo->tableShowsHeaderIcons);
	fuiRect columnSwitchRect = fuiLayoutSlot(ui, columnSwitchWidth);
	(void)fuiCheckbox(ui, columnSwitchRect, "On Kind", &demo->tableIconsOnKindColumn);
	fuiRect textSwitchRect = fuiLayoutSlot(ui, textSwitchWidth);
	(void)fuiCheckbox(ui, textSwitchRect, "Icons only", &demo->tableIconsOnly);
	fuiEndStack(ui);

	float listTop = controlsRect.y + controlsRect.h + DEMO_TABLE_CONTROLS_GAP;
	fuiRect listRect = fuiRectMake(contentRect.x, listTop, contentRect.w, contentRect.y + contentRect.h - listTop);

	// Every row's button says Play except the one that is sounding, which says Stop. The labels are per ROW,
	// so one column can read differently on the row that is doing something.
	fuiColor tintNobodySet = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
	for(int32_t rowIndex = 0; rowIndex < DEMO_TABLE_ROW_COUNT; ++rowIndex) {
		demo->tableIconTintForRow[rowIndex] = tintNobodySet;
		if(demo->tableRowButtons[rowIndex] == fpl_null) {
			continue; // the row that opted out stays opted out
		}
		demo->tableRowButtons[rowIndex] = (rowIndex == demo->tablePlayingRow) ? "Stop" : "Play";
	}

	// The sounding row's icon wears the accent color and every other row's entry stays FULLY TRANSPARENT, which
	// is the way fuiListIcons reads "nobody set this one" and falls back to the tint of the whole sheet.
	if(demo->tablePlayingRow >= 0) {
		const fuiTheme *theme = fuiGetTheme(ui);
		demo->tableIconTintForRow[demo->tablePlayingRow] = theme->accentColor;
	}

	// Which of the two sheets is showing. A sheet that failed to upload is zero, and the switch falls back to
	// the one that did rather than to a list of blank rows.
	bool colorSheetIsUsable = demo->tableUsesColorSheet && (demo->iconSheetColor != 0);
	fuiTextureId sheet = colorSheetIsUsable ? demo->iconSheetColor : demo->iconSheet;
	fuiVec2 sheetSize = colorSheetIsUsable ? demo->iconSheetColorSize : demo->iconSheetSize;
	const int32_t nameColumn = 0;
	const int32_t kindColumn = 1;

	fuiListIcons icons = fplZeroInit;
	icons.sheet = sheet;
	icons.sheetSize = sheetSize;
	icons.columns = (int32_t)DEMO_ICON_CELL_COUNT;
	icons.rows = 1;
	icons.cellForRow = demo->tableIconForRow;
	icons.cellForRowCount = DEMO_TABLE_ROW_COUNT;
	icons.rowScale = DEMO_ICON_ROW_SCALE;
	icons.tintForRow = demo->tableIconTintForRow;
	icons.column = demo->tableIconsOnKindColumn ? kindColumn : nameColumn;
	icons.iconOnly = demo->tableIconsOnly;
	icons.cellForColumn = demo->tableShowsHeaderIcons ? g_demoTableHeaderIcons : fpl_null;
	icons.cellForColumnCount = DEMO_TABLE_COLUMN_COUNT;

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

	Everything the tree reads belongs to the demo - the nodes, the flags saying which folders are open, the table
	saying which icon a row draws. The library writes exactly one of those, the flag of a node whose expander was
	clicked, and copies none of them.
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
		if(fuiButton(ui, expandButtonRect, "Expand all")) {
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, true);
			DemoSay(demo, "All open. Nothing had to be said to the tree: a short one hashes its own flags.");
		}
		fuiRect collapseButtonRect = fuiLayoutSlot(ui, foldButtonWidth);
		if(fuiButton(ui, collapseButtonRect, "Collapse all")) {
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, false);
			DemoSay(demo, "All shut - and the selection stayed put, because it is a NODE and not a row.");
		}
		fuiEndStack(ui);

		fuiRect findButtonRect = fuiLayoutSlot(ui, DEMO_ROW_HEIGHT);
		if(fuiButton(ui, findButtonRect, "Find " DEMO_TREE_BURIED_FILE)) {
			int32_t buriedNode = DemoFindTreeNode(demo, DEMO_TREE_BURIED_FILE);
			// Shut first, so what the two calls underneath do is actually visible rather than already true.
			fuiTreeSetExpandedAll(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, false);
			(void)fuiTreeExpandToNode(demo->treeNodes, demo->treeNodeCount, demo->treeIsExpanded, buriedNode);
			demo->treeSelection = buriedNode;
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

		// The colour sheet when it uploaded, and the coverage one when it did not - a folder that reads as a
		// folder wants two shades inside one glyph, which a single channel cannot carry.
		bool colorSheetIsUsable = (demo->iconSheetColor != 0);
		fuiTextureId sheet = colorSheetIsUsable ? demo->iconSheetColor : demo->iconSheet;
		fuiVec2 sheetSize = colorSheetIsUsable ? demo->iconSheetColorSize : demo->iconSheetSize;

		fuiListIcons icons = fplZeroInit;
		icons.sheet = sheet;
		icons.sheetSize = sheetSize;
		icons.columns = (int32_t)DEMO_ICON_CELL_COUNT;
		icons.rows = 1;
		icons.cellForRow = demo->treeIconForNode;
		icons.cellForRowCount = demo->treeNodeCount;
		icons.rowScale = DEMO_TREE_ICON_ROW_SCALE;

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

//! One cell of the generated sheet, described as a PICTURE rather than as a list icon. A title bar icon is
//! an ordinary fuiImageDesc, so what it wants is the whole sheet plus the slice of it this cell occupies.
//! The colour sheet is preferred when it uploaded, and the coverage one is stained with the tint instead.
static fuiImageDesc DemoIconCellImage(const DemoState *demo, const int32_t cellIndex, const fuiColor coverageTint) {
	bool colorSheetIsUsable = (demo->iconSheetColor != 0);

	const float cellWidthInUv = 1.0f / (float)DEMO_ICON_CELL_COUNT;

	fuiImageDesc result = fplZeroInit;
	result.texture = colorSheetIsUsable ? demo->iconSheetColor : demo->iconSheet;
	result.textureSize = colorSheetIsUsable ? demo->iconSheetColorSize : demo->iconSheetSize;
	result.uvMin = fuiV2((float)cellIndex * cellWidthInUv, 0.0f);
	result.uvMax = fuiV2((float)(cellIndex + 1) * cellWidthInUv, 1.0f);
	result.tint = colorSheetIsUsable ? fuiColorRGBA(1.0f, 1.0f, 1.0f, 1.0f) : coverageTint;
	result.scaleMode = fuiImageScaleMode_Letterbox;
	return(result);
}

//! What the hand built dialog says. Wrapped by the label rather than by the line breaks here, so it reflows
//! as the dialog is dragged wider.
static const char *g_demoAboutText =
	"final_ui.h draws this box, the picture in its caption bar and every widget under it.\n"
	"\n"
	"That picture is one cell of the very same sheet the Entity table and the Project tree draw from - "
	"generated at startup rather than loaded from anywhere - handed over as a plain fuiImageDesc.\n"
	"\n"
	"It is OPTIONAL: fuiBeginModal and fuiBeginModalResizable take no picture at all and are unchanged. "
	"The caption starts beside the icon rather than under it, however long the caption happens to be.";

//! Everything but this one is a STANDARD dialog the library builds whole. This one is built by hand out of
//! fuiBeginModalResizableIcon, which is the only way to reach the title bar icon.
static void BuildAboutDialog(fuiContext *ui, DemoState *demo) {
	const fuiTheme *theme = fuiGetTheme(ui);

	const float aboutWidth = 480.0f;
	const float aboutHeight = 360.0f;
	const float aboutButtonWidth = 96.0f;
	const float aboutButtonHeight = DEMO_ROW_HEIGHT + 4.0f;

	fuiImageDesc aboutIcon = DemoIconCellImage(demo, DEMO_ICON_CELL_LEVEL, theme->accentColor);

	if(fuiBeginModalResizableIcon(ui, "about", "About this demo", aboutWidth, aboutHeight, &aboutIcon)) {
		fuiRect content = fuiLayoutRemaining(ui);

		float textHeight = content.h - aboutButtonHeight - theme->widgetSpacing;
		fuiRect textRect = fuiRectMake(content.x, content.y, content.w, textHeight);
		fuiLabelEx(ui, textRect, g_demoAboutText, true, true);

		float buttonX = content.x + (content.w - aboutButtonWidth) * 0.5f;
		float buttonY = content.y + content.h - aboutButtonHeight;
		bool wasAccepted = fuiButton(ui, fuiRectMake(buttonX, buttonY, aboutButtonWidth, aboutButtonHeight), "OK");

		// Through the library rather than by asking the key plainly, so a dialog stacked under this one does
		// not answer the very same press the moment this one closes.
		bool wasAnsweredByAKey = fuiDialogTakeKey(ui, fuiKey_Return) || fuiDialogTakeKey(ui, fuiKey_Escape);

		if(wasAccepted || wasAnsweredByAKey) {
			fuiCloseDialog(ui, "about");
			DemoSay(demo, "Closed. That dialog was built by hand - the icon in its bar is what the standard ones cannot take.");
		}
	}
	fuiEndModal(ui);
}

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

	BuildAboutDialog(ui, demo);

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

	// Which of the two demos this is, said out loud - FUI_Framework builds the same interface and says its own
	// host here. The right hand items are laid out from the right edge inwards, so this one ends up furthest
	// left of them.
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
// The world behind the interface
// ----------------------------------------------------------------------------

//! A grid, so there is something under the panels to see them float over -- and something fuiWantsMouse
//! can protect. Drawn with the same OpenGL the backend uses, before the interface goes on top.
static void RenderBackdrop(const DemoState *demo, const int32_t windowWidth, const int32_t windowHeight) {
	glViewport(0, 0, (GLsizei)windowWidth, (GLsizei)windowHeight);
	glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if(!demo->showGrid) {
		return;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double)windowWidth, (double)windowHeight, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float spacing = demo->gridSize * demo->zoom;
	if(spacing < 4.0f) {
		spacing = 4.0f;
	}
	glColor4f(demo->tint.r, demo->tint.g, demo->tint.b, 0.10f);
	glBegin(GL_LINES);
	for(float x = 0.0f; x < (float)windowWidth; x += spacing) {
		glVertex2f(x, 0.0f);
		glVertex2f(x, (float)windowHeight);
	}
	for(float y = 0.0f; y < (float)windowHeight; y += spacing) {
		glVertex2f(0.0f, y);
		glVertex2f((float)windowWidth, y);
	}
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	fplSettings settings = fplZeroInit;
	fplSetDefaultSettings(&settings);
	fplCopyString(DEMO_WINDOW_TITLE, settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = DEMO_WINDOW_WIDTH;
	settings.window.windowSize.height = DEMO_WINDOW_HEIGHT;
	settings.video.backend = fplVideoBackendType_OpenGL;
	// Fixed function, because the backend next door is deliberately the smallest one that can exist.
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings.video.isVSync = true;

	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_Video, &settings)) {
		fprintf(stderr, "failed to initialize the platform\n");
		return 1;
	}
	if(!fglLoadOpenGL(true)) {
		fprintf(stderr, "failed to load OpenGL\n");
		fplPlatformRelease();
		return 1;
	}

	// One bake, one upload, and the interface has a font. Nothing is read from disk: the face is one of
	// final_fonts.h's embedded ones, so the demo is a single executable with no asset beside it.
	fuiStbttFont bakedFont;
	fuiStbttBakeSettings bakeSettings = fuiStbttDefaultBakeSettings();
	bakeSettings.pixelHeight = DEMO_FONT_PIXEL_HEIGHT;
	bakeSettings.atlasWidth = DEMO_FONT_ATLAS_SIDE;
	bakeSettings.atlasHeight = DEMO_FONT_ATLAS_SIDE;
	if(!fuiStbttFontBake(&bakedFont, ptr_fontBitstreamVeraRegular, &bakeSettings)) {
		fprintf(stderr, "failed to bake the font\n");
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}

	uint32_t atlasTexture = 0;
	if(!fuiGL1UploadFontAtlas(bakedFont.atlasPixels, bakedFont.atlasWidth, bakedFont.atlasHeight, &atlasTexture)) {
		fprintf(stderr, "failed to upload the font atlas\n");
		fuiStbttFontRelease(&bakedFont);
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}

	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, (fuiTextureId)atlasTexture);
	fuiContext ui;
	if(!fuiInit(&ui, &font, fpl_null)) {
		fprintf(stderr, "failed to initialize the user interface\n");
		fuiGL1DeleteTexture(atlasTexture);
		fuiStbttFontRelease(&bakedFont);
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}

	// Optional, and worth wiring: without it the text field simply has no clipboard.
	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = fuiFplGetClipboardText;
	platform.setClipboardText = fuiFplSetClipboardText;
	fuiSetPlatform(&ui, &platform);

	DemoState demo;
	DemoInit(&demo);

	// The other textures, and the only assets in the demo that are not a font: four icon cells the demo draws
	// itself, once in coverage and once in color. A failed upload leaves that sheet at zero, which is a list of
	// plain text rows rather than a reason not to start.
	//
	// Both sheets are 128 by 32, which is a power of two in BOTH axes on purpose. This backend is OpenGL 1.1
	// with no extension test in it, and a size of any other shape was only ever guaranteed from OpenGL 2.0 on -
	// so a sheet of five 48 pixel cells wants rounding up to 256 by 64 with the spare cells left empty, rather
	// than going up at its natural 240 by 48. A current driver takes either without complaint; the habit is for
	// the old implementations this backend is also meant to run on.
	unsigned char iconPixels[DEMO_ICON_SHEET_WIDTH * DEMO_ICON_SHEET_HEIGHT];
	DemoDrawIconSheet(iconPixels);
	uint32_t iconTexture = 0;
	if(fuiGL1UploadFontAtlas(iconPixels, DEMO_ICON_SHEET_WIDTH, DEMO_ICON_SHEET_HEIGHT, &iconTexture)) {
		demo.iconSheet = (fuiTextureId)iconTexture;
		demo.iconSheetSize = fuiV2((float)DEMO_ICON_SHEET_WIDTH, (float)DEMO_ICON_SHEET_HEIGHT);
	}

	// The colored one takes the OTHER road into the backend: four channels through fuiGL1UploadImageRGBA rather
	// than one through the atlas call. Linear, because the 32 pixel cells are drawn at 44 and a nearest filter
	// would show every step of the scale.
	unsigned char colorIconPixels[DEMO_ICON_SHEET_COLOR_BYTES];
	DemoDrawColorIconSheet(colorIconPixels);
	uint32_t colorIconTexture = 0;
	const bool colorSheetIsFilteredLinearly = true;
	if(fuiGL1UploadImageRGBA(colorIconPixels, DEMO_ICON_SHEET_WIDTH, DEMO_ICON_SHEET_HEIGHT, colorSheetIsFilteredLinearly, &colorIconTexture)) {
		demo.iconSheetColor = (fuiTextureId)colorIconTexture;
		demo.iconSheetColorSize = fuiV2((float)DEMO_ICON_SHEET_WIDTH, (float)DEMO_ICON_SHEET_HEIGHT);
	}

	fuiFplInput bridge;
	fuiFplInputInit(&bridge);

	float smoothedFrameTime = 1.0f / 60.0f;
	while(demo.isRunning && fplWindowUpdate()) {
		fuiFplInputPumpEvents(&bridge);
		fuiFplInputBuild(&bridge);

		// Smoothed, because a per-frame reciprocal flickers too fast to read.
		float frameTime = bridge.input.deltaTime;
		if(frameTime > 0.0f) {
			smoothedFrameTime = smoothedFrameTime * 0.9f + frameTime * 0.1f;
		}
		demo.framesPerSecond = (smoothedFrameTime > 0.0f) ? (1.0f / smoothedFrameTime) : 0.0f;

		// One pass: build the whole interface between begin and end, and let the library work out what
		// was clicked. That is the entire contract.
		fuiBeginFrame(&ui, &bridge.input, fuiPass_Both);
		BuildUserInterface(&ui, &demo, bridge.rightPressedThisFrame);
		fuiEndFrame(&ui);

		// Asked here and nowhere else: this is the first moment the answer is complete for this frame.
		demo.uiOwnedTheMouseLastFrame = fuiWantsMouse(&ui);

		RenderBackdrop(&demo, bridge.input.windowSize.x, bridge.input.windowSize.y);
		fuiGL1Render(fuiGetDrawData(&ui));

		fplVideoFlip();
	}

	fuiRelease(&ui);
	if(iconTexture != 0) {
		fuiGL1DeleteTexture(iconTexture);
	}
	if(colorIconTexture != 0) {
		fuiGL1DeleteTexture(colorIconTexture);
	}
	fuiGL1DeleteTexture(atlasTexture);
	fuiStbttFontRelease(&bakedFont);
	fglUnloadOpenGL();
	fplPlatformRelease();
	return 0;
}
