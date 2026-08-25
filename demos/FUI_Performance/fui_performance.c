/*
Name:
	FUI_Performance

Description:
	A performance workbench for final_ui.h, on FPL and legacy OpenGL.

	Where FUI_Test asks "what does the library look like", this one asks "what does the library COST".
	It fills the widgets that a database-like application leans on - a list view, a list box, a multi
	line text box and a menu tree - with far more data than any hand written demo would ever carry, and
	then measures what that does to a frame.

	Everything here is deliberately hostile to the library:

	  - A list view of up to a million rows across eight columns, sortable by every one of them
	  - A list box of up to half a million rows whose labels are far wider than the box
	  - A multi line text box holding up to two hundred thousand lines
	  - A menu tree of tens of thousands of items, four levels of submenu deep

	The data is random but DETERMINISTIC: one seed, one xorshift, so two runs at the same scale produce
	byte for byte the same strings and two measurements are of the same thing. Nothing is read from disk.

	The measurement is split where the cost actually splits:

	  - Build   the time inside fuiBeginFrame .. fuiEndFrame, which is the library doing layout and text
	  - Render  the time inside fuiGL1Render, which is the backend draining the draw data
	  - Frame   the whole thing including the vsync wait, which is what the user feels

	plus the counters that explain them: draw commands, vertices, indices, text bytes and how much the
	context arena has taken. A frame time history is drawn as a graph, so a spike from a sort or a scroll
	is visible rather than averaged away.

	The graph and the metrics panel cost draw commands of their own, which would show up in the numbers
	they report. Both can be switched off, and the counter for them is reported separately, so a reading
	can be taken of the data widgets ALONE.

Requirements:
	- C99 compiler
	- OpenGL 1.1 (fixed function, which is all the backend here uses)

Build (from the repository root):
	gcc -std=c99 -O2 demos/FUI_Performance/fui_performance.c -I . -I demos/additions -I demos/dependencies -o fui_performance -lm -ldl
	./fui_performance

	Or with cmake:  cmake -S demos/FUI_Performance -B build/fui_performance && cmake --build build/fui_performance

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
#include <stdlib.h>
#include <string.h>

#define PERF_WINDOW_TITLE "final_ui.h performance workbench (FPL + OpenGL)"
#define PERF_WINDOW_WIDTH 1600
#define PERF_WINDOW_HEIGHT 940

// Baked once, above the largest text on screen, so every size drawn is a reduction of the atlas.
#define PERF_FONT_PIXEL_HEIGHT 34.0f
#define PERF_FONT_ATLAS_SIDE 512u

#define PERF_ROW_HEIGHT 24.0f
#define PERF_METRICS_PANEL_WIDTH 340.0f
#define PERF_TAB_STRIP_HEIGHT 28.0f
#define PERF_CONTENT_INSET 8.0f
#define PERF_STATUS_TEXT_MAX 192

// ----------------------------------------------------------------------------
// Scale steps
//
// Every dataset is sized off ONE of these, so a reading is taken at a scale rather than at a pile of
// unrelated numbers. The steps are decades because that is what makes a cost curve readable: a stage
// that goes up by ten when the data goes up by ten is linear, and one that goes up by a hundred is not.
// ----------------------------------------------------------------------------

#define PERF_SCALE_STEP_COUNT 5
static const int32_t g_perfScaleRowCounts[PERF_SCALE_STEP_COUNT] = { 1000, 10000, 100000, 500000, 1000000 };
static const char *const g_perfScaleLabels[PERF_SCALE_STEP_COUNT] = { "1 K", "10 K", "100 K", "500 K", "1 M" };
#define PERF_SCALE_DEFAULT_INDEX 1

// The list box carries half the rows of the table, because its labels are five times as long and the
// point of it is the WIDTH of a row rather than how many there are.
#define PERF_LIST_ROWS_PER_TABLE_ROW 2

#define PERF_TEXT_STEP_COUNT 4
static const int32_t g_perfTextLineCounts[PERF_TEXT_STEP_COUNT] = { 500, 5000, 50000, 200000 };
static const char *const g_perfTextLabels[PERF_TEXT_STEP_COUNT] = { "500", "5 K", "50 K", "200 K" };
#define PERF_TEXT_DEFAULT_INDEX 1

#define PERF_MENU_STEP_COUNT 4
static const int32_t g_perfMenuItemCounts[PERF_MENU_STEP_COUNT] = { 10, 40, 120, 400 };
static const char *const g_perfMenuLabels[PERF_MENU_STEP_COUNT] = { "10", "40", "120", "400" };
#define PERF_MENU_DEFAULT_INDEX 1

// How wide and how deep the generated menu tree is. Only the OPEN path costs anything per frame, but the
// whole tree costs memory and generation time, which is a number worth seeing too.
#define PERF_MENU_TOP_LEVEL_COUNT 12
#define PERF_MENU_SUBMENUS_PER_MENU 3
#define PERF_MENU_MAX_DEPTH 4

#define PERF_TABLE_COLUMN_COUNT 8

// ----------------------------------------------------------------------------
// Deterministic randomness
//
// xorshift32, seeded once. Every string in this program comes out of it, so the same scale always makes
// the same data and two measurements can be compared at all.
// ----------------------------------------------------------------------------

#define PERF_RANDOM_SEED 0x9E3779B9u

typedef struct PerfRandom {
	uint32_t state;
} PerfRandom;

static void PerfRandomSeed(PerfRandom *random, const uint32_t seed) {
	// Zero is the one state xorshift cannot leave, so it is replaced rather than trusted.
	random->state = (seed != 0u) ? seed : PERF_RANDOM_SEED;
}

static uint32_t PerfRandomNext(PerfRandom *random) {
	uint32_t state = random->state;
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	random->state = state;
	return(state);
}

static uint32_t PerfRandomBelow(PerfRandom *random, const uint32_t exclusiveUpperBound) {
	if(exclusiveUpperBound == 0u) {
		return(0u);
	}
	uint32_t drawn = PerfRandomNext(random);
	return(drawn % exclusiveUpperBound);
}

// ----------------------------------------------------------------------------
// A bump allocator for the generated strings
//
// Millions of little strings out of malloc would spend more time in the allocator than in the generator,
// and every one of them would carry a header bigger than the string. One block, one cursor, and the
// pointers handed out stay valid until the whole dataset is thrown away.
// ----------------------------------------------------------------------------

typedef struct PerfStringArena {
	char *bytes;
	size_t capacity;
	size_t used;
	bool ranOutOfRoom;
} PerfStringArena;

static bool PerfStringArenaInit(PerfStringArena *arena, const size_t capacity) {
	memset(arena, 0, sizeof(*arena));
	char *block = (char *)malloc(capacity);
	if(block == fpl_null) {
		return(false);
	}
	arena->bytes = block;
	arena->capacity = capacity;
	arena->used = 0;
	return(true);
}

static void PerfStringArenaRelease(PerfStringArena *arena) {
	if(arena->bytes != fpl_null) {
		free(arena->bytes);
	}
	memset(arena, 0, sizeof(*arena));
}

//! Hands out a writable run of bytes, or null when the block is spent
static char *PerfStringArenaTake(PerfStringArena *arena, const size_t byteCount) {
	size_t wouldBeUsed = arena->used + byteCount;
	if(wouldBeUsed > arena->capacity) {
		arena->ranOutOfRoom = true;
		return(fpl_null);
	}
	char *result = &arena->bytes[arena->used];
	arena->used = wouldBeUsed;
	return(result);
}

// ----------------------------------------------------------------------------
// Hand rolled formatting
//
// snprintf is roughly a microsecond a call once its format string is parsed, and the generator makes six
// strings per row. At a million rows that alone is six seconds of a stall the user watches. These write
// straight into the destination and cost a few nanoseconds each.
// ----------------------------------------------------------------------------

static const char g_perfHexDigits[] = "0123456789abcdef";

//! Appends a decimal number, padded with leading zeroes to at least minimumDigits
static size_t PerfWriteUInt(char *destination, size_t offset, const uint32_t value, const int32_t minimumDigits) {
	char reversedDigits[16];
	int32_t digitCount = 0;
	uint32_t remaining = value;
	do {
		reversedDigits[digitCount++] = (char)('0' + (remaining % 10u));
		remaining /= 10u;
	} while(remaining > 0u && digitCount < (int32_t)sizeof(reversedDigits));

	for(int32_t padIndex = digitCount; padIndex < minimumDigits; ++padIndex) {
		destination[offset++] = '0';
	}
	for(int32_t digitIndex = digitCount - 1; digitIndex >= 0; --digitIndex) {
		destination[offset++] = reversedDigits[digitIndex];
	}
	return(offset);
}

//! Appends a fixed width lowercase hexadecimal number, most significant nibble first
static size_t PerfWriteHex(char *destination, size_t offset, const uint32_t value, const int32_t digitCount) {
	for(int32_t digitIndex = digitCount - 1; digitIndex >= 0; --digitIndex) {
		uint32_t nibble = (value >> (digitIndex * 4)) & 0xFu;
		destination[offset++] = g_perfHexDigits[nibble];
	}
	return(offset);
}

//! Appends a zero terminated string, without its terminator
static size_t PerfWriteText(char *destination, size_t offset, const char *text) {
	size_t textIndex = 0;
	while(text[textIndex] != '\0') {
		destination[offset++] = text[textIndex++];
	}
	return(offset);
}

static size_t PerfWriteChar(char *destination, size_t offset, const char character) {
	destination[offset++] = character;
	return(offset);
}

// ----------------------------------------------------------------------------
// The word pools every generated string is assembled from
//
// Real looking names matter more than they sound: a column of "row 12345" is all the same width and all
// the same prefix, which is the one case a text layout is fastest at. Words of differing lengths that
// share prefixes are what a real table looks like, and what a sort has to work at.
// ----------------------------------------------------------------------------

static const char *const g_perfFirstWords[] = {
	"azure", "crimson", "gilded", "hollow", "iron", "jade", "lunar", "marble",
	"nether", "obsidian", "pale", "quartz", "russet", "sable", "tidal", "umber",
	"verdant", "wisp", "xenon", "yarrow", "zephyr", "amber", "bronze", "cobalt",
};

static const char *const g_perfSecondWords[] = {
	"bastion", "cavern", "delta", "ember", "forge", "grove", "harbor", "isle",
	"junction", "keep", "lantern", "marsh", "node", "outpost", "pillar", "quarry",
	"ridge", "spire", "tower", "vault", "warren", "yard", "zone", "anchor",
};

static const char *const g_perfCategories[] = {
	"Texture", "Mesh", "Sound", "Script", "Shader", "Material", "Animation", "Prefab", "Level", "Font",
};

static const char *const g_perfStatuses[] = {
	"OK", "Modified", "Missing", "Locked", "Conflict", "Stale", "Queued", "Building",
};

static const char *const g_perfFolders[] = {
	"assets", "content", "source", "cache", "shared", "vendor",
};

static const fuiColumn g_perfTableColumns[PERF_TABLE_COLUMN_COUNT] = {
	{ "Id", 90.0f },
	{ "Name", 190.0f },
	{ "Category", 110.0f },
	{ "Status", 100.0f },
	{ "Modified", 160.0f },
	{ "Size", 100.0f },
	{ "Path", 380.0f },
	{ "Hash", 160.0f },
};

// ----------------------------------------------------------------------------
// The dataset
// ----------------------------------------------------------------------------

//! One node of the generated menu tree. A node with children is a submenu, one without is a plain row
typedef struct PerfMenuNode {
	const char *label;
	struct PerfMenuNode *children;
	int32_t childCount;
} PerfMenuNode;

typedef struct PerfDataSet {
	PerfStringArena strings;

	//! Row major, PERF_TABLE_COLUMN_COUNT entries per row, which is the layout fuiListView takes
	const char **tableCells;
	int32_t tableRowCount;

	const char **listItems;
	int32_t listItemCount;

	//! One flat buffer of newline separated lines, which is what a multiline text field takes
	char *textBuffer;
	size_t textCapacity;
	int32_t textLineCount;

	//! Every node of the menu tree, sub-allocated from one block so a subtree is a contiguous run
	PerfMenuNode *menuNodes;
	int32_t menuNodeCapacity;
	int32_t menuNodeCount;
	PerfMenuNode *menuTopLevel;
	int32_t menuTopLevelCount;
	int32_t menuItemsPerMenu;

	//! What it took to build all of the above, which is a cost of its own worth seeing
	double generationMilliseconds;
	size_t generatedByteCount;
	bool isComplete;
} PerfDataSet;

static void PerfDataSetRelease(PerfDataSet *data) {
	PerfStringArenaRelease(&data->strings);
	if(data->tableCells != fpl_null) {
		free(data->tableCells);
	}
	if(data->listItems != fpl_null) {
		free(data->listItems);
	}
	if(data->textBuffer != fpl_null) {
		free(data->textBuffer);
	}
	if(data->menuNodes != fpl_null) {
		free(data->menuNodes);
	}
	memset(data, 0, sizeof(*data));
}

//! How many menus the tree holds, which is what tells the generator how many nodes to reserve
static int32_t PerfCountMenusInTree(void) {
	int32_t menusAtThisDepth = PERF_MENU_TOP_LEVEL_COUNT;
	int32_t totalMenus = 0;
	for(int32_t depth = 1; depth <= PERF_MENU_MAX_DEPTH; ++depth) {
		totalMenus += menusAtThisDepth;
		menusAtThisDepth *= PERF_MENU_SUBMENUS_PER_MENU;
	}
	return(totalMenus);
}

//! Reserves a contiguous run of nodes, which is what makes a menu's children one array rather than a list
static PerfMenuNode *PerfTakeMenuNodes(PerfDataSet *data, const int32_t nodeCount) {
	int32_t wouldBeCount = data->menuNodeCount + nodeCount;
	if(wouldBeCount > data->menuNodeCapacity) {
		return(fpl_null);
	}
	PerfMenuNode *result = &data->menuNodes[data->menuNodeCount];
	data->menuNodeCount = wouldBeCount;
	return(result);
}

//! Fills one menu's children, recursing into the first few of them while there is depth left to spend
static void PerfGenerateMenuChildren(PerfDataSet *data, PerfRandom *random, PerfMenuNode *menu, const int32_t depth) {
	int32_t itemCount = data->menuItemsPerMenu;
	PerfMenuNode *children = PerfTakeMenuNodes(data, itemCount);
	if(children == fpl_null) {
		menu->children = fpl_null;
		menu->childCount = 0;
		return;
	}
	menu->children = children;
	menu->childCount = itemCount;

	// Only the first few children become submenus, so the tree widens at a rate the node budget survives.
	int32_t submenuCount = (depth < PERF_MENU_MAX_DEPTH) ? PERF_MENU_SUBMENUS_PER_MENU : 0;

	const size_t menuLabelCapacity = 48;
	for(int32_t itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
		PerfMenuNode *child = &children[itemIndex];
		child->children = fpl_null;
		child->childCount = 0;

		char *label = PerfStringArenaTake(&data->strings, menuLabelCapacity);
		if(label == fpl_null) {
			child->label = "<out of memory>";
			continue;
		}
		uint32_t firstWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfFirstWords));
		uint32_t secondWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		size_t offset = 0;
		if(itemIndex < submenuCount) {
			offset = PerfWriteText(label, offset, "More ");
		}
		offset = PerfWriteText(label, offset, g_perfFirstWords[firstWordIndex]);
		offset = PerfWriteChar(label, offset, ' ');
		offset = PerfWriteText(label, offset, g_perfSecondWords[secondWordIndex]);
		offset = PerfWriteChar(label, offset, ' ');
		offset = PerfWriteUInt(label, offset, (uint32_t)itemIndex, 3);
		label[offset] = '\0';
		child->label = label;

		if(itemIndex < submenuCount) {
			PerfGenerateMenuChildren(data, random, child, depth + 1);
		}
	}
}

static void PerfGenerateMenuTree(PerfDataSet *data, PerfRandom *random) {
	PerfMenuNode *topLevel = PerfTakeMenuNodes(data, PERF_MENU_TOP_LEVEL_COUNT);
	if(topLevel == fpl_null) {
		return;
	}
	data->menuTopLevel = topLevel;
	data->menuTopLevelCount = PERF_MENU_TOP_LEVEL_COUNT;

	const size_t topLabelCapacity = 32;
	for(int32_t menuIndex = 0; menuIndex < PERF_MENU_TOP_LEVEL_COUNT; ++menuIndex) {
		PerfMenuNode *menu = &topLevel[menuIndex];
		char *label = PerfStringArenaTake(&data->strings, topLabelCapacity);
		if(label == fpl_null) {
			menu->label = "<out of memory>";
			menu->children = fpl_null;
			menu->childCount = 0;
			continue;
		}
		uint32_t wordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		size_t offset = 0;
		offset = PerfWriteText(label, offset, g_perfSecondWords[wordIndex]);
		offset = PerfWriteChar(label, offset, ' ');
		offset = PerfWriteUInt(label, offset, (uint32_t)menuIndex, 2);
		label[offset] = '\0';
		menu->label = label;

		const int32_t topLevelDepth = 1;
		PerfGenerateMenuChildren(data, random, menu, topLevelDepth);
	}
}

// ----------------------------------------------------------------------------
// Row generation
//
// The byte budgets below are what the arena is sized from, so every one of them has to be an upper bound
// on what the writer below it can produce. They are generous rather than exact: a few wasted bytes a row
// is nothing next to being wrong by one and writing into the next string.
// ----------------------------------------------------------------------------

#define PERF_CELL_ID_CAPACITY 12
#define PERF_CELL_NAME_CAPACITY 32
#define PERF_CELL_MODIFIED_CAPACITY 20
#define PERF_CELL_SIZE_CAPACITY 16
#define PERF_CELL_PATH_CAPACITY 88
#define PERF_CELL_HASH_CAPACITY 20
#define PERF_CELL_BYTES_PER_ROW (PERF_CELL_ID_CAPACITY + PERF_CELL_NAME_CAPACITY + PERF_CELL_MODIFIED_CAPACITY + PERF_CELL_SIZE_CAPACITY + PERF_CELL_PATH_CAPACITY + PERF_CELL_HASH_CAPACITY)

#define PERF_LIST_ITEM_CAPACITY 128
#define PERF_MENU_LABEL_CAPACITY 48
#define PERF_MENU_TOP_LABEL_CAPACITY 32
#define PERF_TEXT_BYTES_PER_LINE 128

// Which columns of the table hold what, so the writers below and the sort default agree on one answer
#define PERF_COLUMN_ID 0
#define PERF_COLUMN_NAME 1
#define PERF_COLUMN_CATEGORY 2
#define PERF_COLUMN_STATUS 3
#define PERF_COLUMN_MODIFIED 4
#define PERF_COLUMN_SIZE 5
#define PERF_COLUMN_PATH 6
#define PERF_COLUMN_HASH 7

#define PERF_YEAR_FIRST 2019u
#define PERF_YEAR_SPAN 7u
#define PERF_MONTHS_PER_YEAR 12u
#define PERF_DAYS_PER_MONTH 28u
#define PERF_HOURS_PER_DAY 24u
#define PERF_MINUTES_PER_HOUR 60u

//! Writes a size that reads like one, so the column has short and long values in it rather than one shape
static size_t PerfWriteSize(char *destination, size_t offset, PerfRandom *random) {
	static const char *const unitNames[] = { "B", "KB", "MB", "GB" };
	uint32_t unitIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(unitNames));
	uint32_t wholePart = PerfRandomBelow(random, 1024u);
	uint32_t fractionPart = PerfRandomBelow(random, 10u);
	offset = PerfWriteUInt(destination, offset, wholePart, 1);
	offset = PerfWriteChar(destination, offset, '.');
	offset = PerfWriteUInt(destination, offset, fractionPart, 1);
	offset = PerfWriteChar(destination, offset, ' ');
	offset = PerfWriteText(destination, offset, unitNames[unitIndex]);
	return(offset);
}

static size_t PerfWriteTimestamp(char *destination, size_t offset, PerfRandom *random) {
	uint32_t year = PERF_YEAR_FIRST + PerfRandomBelow(random, PERF_YEAR_SPAN);
	uint32_t month = 1u + PerfRandomBelow(random, PERF_MONTHS_PER_YEAR);
	uint32_t day = 1u + PerfRandomBelow(random, PERF_DAYS_PER_MONTH);
	uint32_t hour = PerfRandomBelow(random, PERF_HOURS_PER_DAY);
	uint32_t minute = PerfRandomBelow(random, PERF_MINUTES_PER_HOUR);
	offset = PerfWriteUInt(destination, offset, year, 4);
	offset = PerfWriteChar(destination, offset, '-');
	offset = PerfWriteUInt(destination, offset, month, 2);
	offset = PerfWriteChar(destination, offset, '-');
	offset = PerfWriteUInt(destination, offset, day, 2);
	offset = PerfWriteChar(destination, offset, ' ');
	offset = PerfWriteUInt(destination, offset, hour, 2);
	offset = PerfWriteChar(destination, offset, ':');
	offset = PerfWriteUInt(destination, offset, minute, 2);
	return(offset);
}

static bool PerfGenerateTable(PerfDataSet *data, PerfRandom *random, const int32_t rowCount) {
	size_t cellCount = (size_t)rowCount * (size_t)PERF_TABLE_COLUMN_COUNT;
	const char **cells = (const char **)malloc(cellCount * sizeof(const char *));
	if(cells == fpl_null) {
		return(false);
	}
	data->tableCells = cells;
	data->tableRowCount = rowCount;

	for(int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		uint32_t firstWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfFirstWords));
		uint32_t secondWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		uint32_t categoryIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfCategories));
		uint32_t statusIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfStatuses));
		uint32_t folderIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfFolders));
		const char *firstWord = g_perfFirstWords[firstWordIndex];
		const char *secondWord = g_perfSecondWords[secondWordIndex];
		uint32_t nameNumber = PerfRandomBelow(random, 10000u);

		char *idCell = PerfStringArenaTake(&data->strings, PERF_CELL_ID_CAPACITY);
		char *nameCell = PerfStringArenaTake(&data->strings, PERF_CELL_NAME_CAPACITY);
		char *modifiedCell = PerfStringArenaTake(&data->strings, PERF_CELL_MODIFIED_CAPACITY);
		char *sizeCell = PerfStringArenaTake(&data->strings, PERF_CELL_SIZE_CAPACITY);
		char *pathCell = PerfStringArenaTake(&data->strings, PERF_CELL_PATH_CAPACITY);
		char *hashCell = PerfStringArenaTake(&data->strings, PERF_CELL_HASH_CAPACITY);
		bool everyCellArrived = (idCell != fpl_null) && (nameCell != fpl_null) && (modifiedCell != fpl_null) && (sizeCell != fpl_null) && (pathCell != fpl_null) && (hashCell != fpl_null);
		if(!everyCellArrived) {
			return(false);
		}

		size_t offset = 0;
		offset = PerfWriteChar(idCell, offset, '#');
		offset = PerfWriteUInt(idCell, offset, (uint32_t)rowIndex, 7);
		idCell[offset] = '\0';

		offset = 0;
		offset = PerfWriteText(nameCell, offset, firstWord);
		offset = PerfWriteChar(nameCell, offset, '-');
		offset = PerfWriteText(nameCell, offset, secondWord);
		offset = PerfWriteChar(nameCell, offset, '-');
		offset = PerfWriteUInt(nameCell, offset, nameNumber, 4);
		nameCell[offset] = '\0';

		offset = 0;
		offset = PerfWriteTimestamp(modifiedCell, offset, random);
		modifiedCell[offset] = '\0';

		offset = 0;
		offset = PerfWriteSize(sizeCell, offset, random);
		sizeCell[offset] = '\0';

		// Deliberately long, and the one column that is wider than any sane default width. A cell whose
		// text runs off the end of its column is the case a text layout has the most work to throw away.
		offset = 0;
		offset = PerfWriteChar(pathCell, offset, '/');
		offset = PerfWriteText(pathCell, offset, g_perfFolders[folderIndex]);
		offset = PerfWriteChar(pathCell, offset, '/');
		offset = PerfWriteText(pathCell, offset, firstWord);
		offset = PerfWriteChar(pathCell, offset, '/');
		offset = PerfWriteText(pathCell, offset, secondWord);
		offset = PerfWriteChar(pathCell, offset, '/');
		offset = PerfWriteText(pathCell, offset, nameCell);
		offset = PerfWriteText(pathCell, offset, ".dat");
		pathCell[offset] = '\0';

		uint32_t hashHigh = PerfRandomNext(random);
		uint32_t hashLow = PerfRandomNext(random);
		offset = 0;
		offset = PerfWriteHex(hashCell, offset, hashHigh, 8);
		offset = PerfWriteHex(hashCell, offset, hashLow, 8);
		hashCell[offset] = '\0';

		size_t cellBase = (size_t)rowIndex * (size_t)PERF_TABLE_COLUMN_COUNT;
		cells[cellBase + PERF_COLUMN_ID] = idCell;
		cells[cellBase + PERF_COLUMN_NAME] = nameCell;
		// Category and status come out of a small pool, so those two columns cost no arena at all - which
		// is what a real database column of an enumeration looks like.
		cells[cellBase + PERF_COLUMN_CATEGORY] = g_perfCategories[categoryIndex];
		cells[cellBase + PERF_COLUMN_STATUS] = g_perfStatuses[statusIndex];
		cells[cellBase + PERF_COLUMN_MODIFIED] = modifiedCell;
		cells[cellBase + PERF_COLUMN_SIZE] = sizeCell;
		cells[cellBase + PERF_COLUMN_PATH] = pathCell;
		cells[cellBase + PERF_COLUMN_HASH] = hashCell;
	}
	return(true);
}

static bool PerfGenerateListBox(PerfDataSet *data, PerfRandom *random, const int32_t itemCount) {
	const char **items = (const char **)malloc((size_t)itemCount * sizeof(const char *));
	if(items == fpl_null) {
		return(false);
	}
	data->listItems = items;
	data->listItemCount = itemCount;

	for(int32_t itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
		char *item = PerfStringArenaTake(&data->strings, PERF_LIST_ITEM_CAPACITY);
		if(item == fpl_null) {
			return(false);
		}
		uint32_t firstWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfFirstWords));
		uint32_t secondWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		uint32_t tailWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		uint32_t categoryIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfCategories));
		uint32_t statusIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfStatuses));
		uint32_t tag = PerfRandomNext(random);

		size_t offset = 0;
		offset = PerfWriteChar(item, offset, '[');
		offset = PerfWriteUInt(item, offset, (uint32_t)itemIndex, 7);
		offset = PerfWriteText(item, offset, "] ");
		offset = PerfWriteText(item, offset, g_perfFirstWords[firstWordIndex]);
		offset = PerfWriteChar(item, offset, '-');
		offset = PerfWriteText(item, offset, g_perfSecondWords[secondWordIndex]);
		offset = PerfWriteText(item, offset, " | ");
		offset = PerfWriteText(item, offset, g_perfCategories[categoryIndex]);
		offset = PerfWriteText(item, offset, " | ");
		offset = PerfWriteText(item, offset, g_perfStatuses[statusIndex]);
		offset = PerfWriteText(item, offset, " | ");
		offset = PerfWriteText(item, offset, g_perfSecondWords[tailWordIndex]);
		offset = PerfWriteText(item, offset, " #");
		offset = PerfWriteHex(item, offset, tag, 6);
		item[offset] = '\0';
		items[itemIndex] = item;
	}
	return(true);
}

static bool PerfGenerateText(PerfDataSet *data, PerfRandom *random, const int32_t lineCount) {
	size_t capacity = (size_t)lineCount * (size_t)PERF_TEXT_BYTES_PER_LINE + 1u;
	char *buffer = (char *)malloc(capacity);
	if(buffer == fpl_null) {
		return(false);
	}
	data->textBuffer = buffer;
	data->textCapacity = capacity;
	data->textLineCount = lineCount;

	size_t offset = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		uint32_t firstWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfFirstWords));
		uint32_t secondWordIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfSecondWords));
		uint32_t categoryIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfCategories));
		uint32_t statusIndex = PerfRandomBelow(random, (uint32_t)fplArrayCount(g_perfStatuses));

		offset = PerfWriteUInt(buffer, offset, (uint32_t)lineIndex, 6);
		offset = PerfWriteText(buffer, offset, "  ");
		offset = PerfWriteTimestamp(buffer, offset, random);
		offset = PerfWriteText(buffer, offset, "  ");
		offset = PerfWriteText(buffer, offset, g_perfStatuses[statusIndex]);
		offset = PerfWriteText(buffer, offset, "  ");
		offset = PerfWriteText(buffer, offset, g_perfCategories[categoryIndex]);
		offset = PerfWriteText(buffer, offset, "  ");
		offset = PerfWriteText(buffer, offset, g_perfFirstWords[firstWordIndex]);
		offset = PerfWriteChar(buffer, offset, '-');
		offset = PerfWriteText(buffer, offset, g_perfSecondWords[secondWordIndex]);
		if(lineIndex + 1 < lineCount) {
			offset = PerfWriteChar(buffer, offset, '\n');
		}
	}
	buffer[offset] = '\0';
	return(true);
}

//! Throws the old dataset away and builds a fresh one at the given scale. Returns false when the machine
//! could not find the memory, which leaves an EMPTY dataset rather than a half filled one
static bool PerfDataSetBuild(PerfDataSet *data, const int32_t tableRowCount, const int32_t textLineCount, const int32_t menuItemsPerMenu) {
	PerfDataSetRelease(data);

	fplTimestamp generationStart = fplTimestampQuery();

	int32_t listItemCount = tableRowCount / PERF_LIST_ROWS_PER_TABLE_ROW;
	int32_t menuCount = PerfCountMenusInTree();
	int32_t menuNodeCapacity = PERF_MENU_TOP_LEVEL_COUNT + menuCount * menuItemsPerMenu;

	size_t tableStringBytes = (size_t)tableRowCount * (size_t)PERF_CELL_BYTES_PER_ROW;
	size_t listStringBytes = (size_t)listItemCount * (size_t)PERF_LIST_ITEM_CAPACITY;
	size_t menuStringBytes = (size_t)menuNodeCapacity * (size_t)PERF_MENU_LABEL_CAPACITY + (size_t)PERF_MENU_TOP_LEVEL_COUNT * (size_t)PERF_MENU_TOP_LABEL_CAPACITY;
	size_t arenaCapacity = tableStringBytes + listStringBytes + menuStringBytes;

	if(!PerfStringArenaInit(&data->strings, arenaCapacity)) {
		PerfDataSetRelease(data);
		return(false);
	}

	data->menuNodes = (PerfMenuNode *)malloc((size_t)menuNodeCapacity * sizeof(PerfMenuNode));
	if(data->menuNodes == fpl_null) {
		PerfDataSetRelease(data);
		return(false);
	}
	data->menuNodeCapacity = menuNodeCapacity;
	data->menuNodeCount = 0;
	data->menuItemsPerMenu = menuItemsPerMenu;

	PerfRandom random;
	PerfRandomSeed(&random, PERF_RANDOM_SEED);

	bool tableArrived = PerfGenerateTable(data, &random, tableRowCount);
	bool listArrived = tableArrived && PerfGenerateListBox(data, &random, listItemCount);
	bool textArrived = listArrived && PerfGenerateText(data, &random, textLineCount);
	if(!textArrived) {
		PerfDataSetRelease(data);
		return(false);
	}
	PerfGenerateMenuTree(data, &random);

	fplTimestamp generationEnd = fplTimestampQuery();
	double elapsedSeconds = fplTimestampElapsed(generationStart, generationEnd);
	const double millisecondsPerSecond = 1000.0;
	data->generationMilliseconds = elapsedSeconds * millisecondsPerSecond;

	size_t cellPointerBytes = (size_t)tableRowCount * (size_t)PERF_TABLE_COLUMN_COUNT * sizeof(const char *);
	size_t itemPointerBytes = (size_t)listItemCount * sizeof(const char *);
	size_t menuNodeBytes = (size_t)menuNodeCapacity * sizeof(PerfMenuNode);
	data->generatedByteCount = data->strings.used + cellPointerBytes + itemPointerBytes + menuNodeBytes + data->textCapacity;
	data->isComplete = true;
	return(true);
}

// ----------------------------------------------------------------------------
// Measurement
// ----------------------------------------------------------------------------

#define PERF_HISTORY_COUNT 120
#define PERF_SMOOTHING_WEIGHT 0.92
#define PERF_MILLISECONDS_PER_SECOND 1000.0

typedef struct PerfMetrics {
	//! What this frame cost, unsmoothed, which is what the graph and the worst case read
	double buildMilliseconds;
	double renderMilliseconds;
	double frameMilliseconds;
	//! The same three run through an exponential average, because a per frame number flickers too fast to read
	double smoothedBuildMilliseconds;
	double smoothedRenderMilliseconds;
	double smoothedFrameMilliseconds;
	//! The worst frame since the last reset, which is where a sort or a regeneration shows up
	double worstFrameMilliseconds;
	//! A ring of recent frame times, oldest at historyWriteIndex
	float frameHistory[PERF_HISTORY_COUNT];
	int32_t historyWriteIndex;

	//! Straight out of fuiDrawData, which is what explains the two times above
	uint32_t commandCount;
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t textByteCount;
	//! What the context arena has taken since it was created, which only ever grows
	size_t arenaByteCount;
	//! How many menu rows the open popups really emitted this frame
	uint32_t menuRowCount;
} PerfMetrics;

static void PerfMetricsReset(PerfMetrics *metrics) {
	memset(metrics, 0, sizeof(*metrics));
}

static void PerfMetricsPush(PerfMetrics *metrics, const double buildMilliseconds, const double renderMilliseconds, const double frameMilliseconds) {
	metrics->buildMilliseconds = buildMilliseconds;
	metrics->renderMilliseconds = renderMilliseconds;
	metrics->frameMilliseconds = frameMilliseconds;

	const double keepWeight = PERF_SMOOTHING_WEIGHT;
	const double newWeight = 1.0 - PERF_SMOOTHING_WEIGHT;
	metrics->smoothedBuildMilliseconds = metrics->smoothedBuildMilliseconds * keepWeight + buildMilliseconds * newWeight;
	metrics->smoothedRenderMilliseconds = metrics->smoothedRenderMilliseconds * keepWeight + renderMilliseconds * newWeight;
	metrics->smoothedFrameMilliseconds = metrics->smoothedFrameMilliseconds * keepWeight + frameMilliseconds * newWeight;

	if(frameMilliseconds > metrics->worstFrameMilliseconds) {
		metrics->worstFrameMilliseconds = frameMilliseconds;
	}

	metrics->frameHistory[metrics->historyWriteIndex] = (float)frameMilliseconds;
	metrics->historyWriteIndex = (metrics->historyWriteIndex + 1) % PERF_HISTORY_COUNT;
}

// ----------------------------------------------------------------------------
// Application state
// ----------------------------------------------------------------------------

#define PERF_TAB_LIST_VIEW 0
#define PERF_TAB_LIST_BOX 1
#define PERF_TAB_TEXT_BOX 2
#define PERF_TAB_EVERYTHING 3

static const char *const g_perfTabNames[] = { "List view", "List box", "Text box", "All at once" };

typedef struct PerfState {
	PerfDataSet data;
	PerfMetrics metrics;

	//! What the data was last built at
	int32_t scaleStepIndex;
	int32_t textStepIndex;
	int32_t menuStepIndex;
	//! What the user asked for. Applied BETWEEN frames, never during a build: the list view holds the cell
	//! array for the length of the call, and freeing it out from under one would be a use after free
	int32_t requestedScaleStepIndex;
	int32_t requestedTextStepIndex;
	int32_t requestedMenuStepIndex;

	int32_t activeTab;
	int32_t tableSelection;
	int32_t listSelection;

	bool showMetricsPanel;
	bool showGraph;
	bool sortIsEnabled;
	bool wordWrapIsOn;
	bool uiOwnedTheMouseLastFrame;
	bool isRunning;
	bool ranOutOfMemory;

	char statusMessage[PERF_STATUS_TEXT_MAX];
	//! Counted while the menu tree is built, which is the only way to know what an open popup really emitted
	uint32_t menuRowsThisFrame;
} PerfState;

static void PerfSay(PerfState *state, const char *message) {
	fplCopyString(message, state->statusMessage, fplArrayCount(state->statusMessage));
}

//! Rebuilds the dataset at whatever the user last asked for, and says what it cost
static void PerfApplyRequestedScale(PerfState *state) {
	bool scaleChanged = (state->requestedScaleStepIndex != state->scaleStepIndex);
	bool textChanged = (state->requestedTextStepIndex != state->textStepIndex);
	bool menuChanged = (state->requestedMenuStepIndex != state->menuStepIndex);
	if(!scaleChanged && !textChanged && !menuChanged && state->data.isComplete) {
		return;
	}

	int32_t tableRowCount = g_perfScaleRowCounts[state->requestedScaleStepIndex];
	int32_t textLineCount = g_perfTextLineCounts[state->requestedTextStepIndex];
	int32_t menuItemsPerMenu = g_perfMenuItemCounts[state->requestedMenuStepIndex];

	bool wasBuilt = PerfDataSetBuild(&state->data, tableRowCount, textLineCount, menuItemsPerMenu);
	if(!wasBuilt) {
		state->ranOutOfMemory = true;
		// The request is rolled back rather than retried, so the next frame does not try the same
		// allocation again and stall for as long a second time.
		state->requestedScaleStepIndex = state->scaleStepIndex;
		state->requestedTextStepIndex = state->textStepIndex;
		state->requestedMenuStepIndex = state->menuStepIndex;
		PerfSay(state, "Out of memory at that scale. Nothing was changed - step back down.");
		return;
	}

	state->scaleStepIndex = state->requestedScaleStepIndex;
	state->textStepIndex = state->requestedTextStepIndex;
	state->menuStepIndex = state->requestedMenuStepIndex;
	state->tableSelection = -1;
	state->listSelection = -1;
	state->ranOutOfMemory = false;
	PerfMetricsReset(&state->metrics);

	const double bytesPerMegabyte = 1024.0 * 1024.0;
	double megabytes = (double)state->data.generatedByteCount / bytesPerMegabyte;
	char message[PERF_STATUS_TEXT_MAX];
	fplStringFormat(message, fplArrayCount(message), "Generated %d rows, %d lines and %d menu nodes in %.0f ms (%.1f MB)", state->data.tableRowCount, state->data.textLineCount, state->data.menuNodeCount, state->data.generationMilliseconds, megabytes);
	PerfSay(state, message);
}

static void PerfInit(PerfState *state) {
	memset(state, 0, sizeof(*state));
	state->scaleStepIndex = -1;
	state->textStepIndex = -1;
	state->menuStepIndex = -1;
	state->requestedScaleStepIndex = PERF_SCALE_DEFAULT_INDEX;
	state->requestedTextStepIndex = PERF_TEXT_DEFAULT_INDEX;
	state->requestedMenuStepIndex = PERF_MENU_DEFAULT_INDEX;
	state->activeTab = PERF_TAB_LIST_VIEW;
	state->tableSelection = -1;
	state->listSelection = -1;
	state->showMetricsPanel = true;
	state->showGraph = true;
	state->sortIsEnabled = true;
	state->wordWrapIsOn = false;
	state->isRunning = true;
	PerfSay(state, "Step the scale up with the tool strip and watch what build time does.");
}

// ----------------------------------------------------------------------------
// The menu tree
//
// A closed submenu costs one fuiBeginMenu that answers false, so the tree below a shut menu is never
// walked at all. What DOES cost is an open popup, which emits every one of its rows whether it fits on
// the screen or not - which is exactly what the row counter in the status bar is there to show.
// ----------------------------------------------------------------------------

static void PerfBuildMenuNode(fuiContext *ui, PerfState *state, const PerfMenuNode *node, const int32_t depth) {
	bool isSubmenu = (node->childCount > 0);
	if(isSubmenu) {
		// A submenu row inside an open popup is a row like any other. A top level title is not, so it is
		// not counted: it lives on the bar and is there whether anything is open or not.
		if(depth > 0) {
			state->menuRowsThisFrame += 1u;
		}
		bool isOpen = fuiBeginMenu(ui, node->label);
		if(isOpen) {
			for(int32_t childIndex = 0; childIndex < node->childCount; ++childIndex) {
				PerfBuildMenuNode(ui, state, &node->children[childIndex], depth + 1);
			}
		}
		fuiEndMenu(ui);
		return;
	}

	state->menuRowsThisFrame += 1u;
	bool wasClicked = fuiMenuItem(ui, node->label, fpl_null, true);
	if(wasClicked) {
		char message[PERF_STATUS_TEXT_MAX];
		fplStringFormat(message, fplArrayCount(message), "Chose the menu row \"%s\"", node->label);
		PerfSay(state, message);
	}
}

static void PerfBuildMenuBar(fuiContext *ui, PerfState *state, const fuiRect barRect) {
	fuiBeginMenuBar(ui, "menubar", barRect);

	// A small hand written menu first, so the workbench stays controllable without going through the
	// generated tree to find a switch.
	bool viewIsOpen = fuiBeginMenu(ui, "Workbench");
	if(viewIsOpen) {
		if(fuiMenuItemCheck(ui, "Metrics panel", state->showMetricsPanel, true)) {
			state->showMetricsPanel = !state->showMetricsPanel;
		}
		if(fuiMenuItemCheck(ui, "Frame time graph", state->showGraph, true)) {
			state->showGraph = !state->showGraph;
		}
		if(fuiMenuItemCheck(ui, "Sortable columns", state->sortIsEnabled, true)) {
			state->sortIsEnabled = !state->sortIsEnabled;
		}
		if(fuiMenuItemCheck(ui, "Word wrap in the text box", state->wordWrapIsOn, true)) {
			state->wordWrapIsOn = !state->wordWrapIsOn;
		}
		fuiMenuSeparator(ui);
		if(fuiMenuItem(ui, "Reset the worst frame", fpl_null, true)) {
			state->metrics.worstFrameMilliseconds = 0.0;
			PerfSay(state, "Worst frame reset.");
		}
		fuiMenuSeparator(ui);
		if(fuiMenuItem(ui, "Quit", "Esc", true)) {
			state->isRunning = false;
		}
	}
	fuiEndMenu(ui);

	const int32_t topLevelDepth = 0;
	for(int32_t menuIndex = 0; menuIndex < state->data.menuTopLevelCount; ++menuIndex) {
		const PerfMenuNode *menu = &state->data.menuTopLevel[menuIndex];
		PerfBuildMenuNode(ui, state, menu, topLevelDepth);
	}

	fuiEndMenuBar(ui);
}

// ----------------------------------------------------------------------------
// The tool strip
// ----------------------------------------------------------------------------

static void PerfBuildToolStrip(fuiContext *ui, PerfState *state, const fuiRect stripRect) {
	fuiBeginToolStrip(ui, "toolstrip", stripRect, FUI_AXIS_HORIZONTAL);

	for(int32_t stepIndex = 0; stepIndex < PERF_SCALE_STEP_COUNT; ++stepIndex) {
		bool isTheCurrentStep = (state->scaleStepIndex == stepIndex);
		bool wasClicked = fuiToolStripToggle(ui, g_perfScaleLabels[stepIndex], isTheCurrentStep, true);
		if(wasClicked) {
			state->requestedScaleStepIndex = stepIndex;
		}
	}

	fuiToolStripSeparator(ui);

	bool metricsWasClicked = fuiToolStripToggle(ui, "Metrics", state->showMetricsPanel, true);
	if(metricsWasClicked) {
		state->showMetricsPanel = !state->showMetricsPanel;
	}
	bool graphWasClicked = fuiToolStripToggle(ui, "Graph", state->showGraph, true);
	if(graphWasClicked) {
		state->showGraph = !state->showGraph;
	}
	bool sortWasClicked = fuiToolStripToggle(ui, "Sortable", state->sortIsEnabled, true);
	if(sortWasClicked) {
		state->sortIsEnabled = !state->sortIsEnabled;
	}
	bool wrapWasClicked = fuiToolStripToggle(ui, "Word wrap", state->wordWrapIsOn, true);
	if(wrapWasClicked) {
		state->wordWrapIsOn = !state->wordWrapIsOn;
	}

	fuiToolStripSeparator(ui);

	bool regenerateWasClicked = fuiToolStripButton(ui, "Regenerate");
	if(regenerateWasClicked) {
		// Forced through by pretending nothing is built, which is what makes the same scale rebuild and
		// puts the generation cost back on the clock.
		state->data.isComplete = false;
	}
	bool resetWasClicked = fuiToolStripButton(ui, "Reset peak");
	if(resetWasClicked) {
		state->metrics.worstFrameMilliseconds = 0.0;
	}

	fuiEndToolStrip(ui);
}

// ----------------------------------------------------------------------------
// The metrics panel
// ----------------------------------------------------------------------------

#define PERF_GRAPH_HEIGHT 96.0f
#define PERF_GRAPH_FLOOR_MILLISECONDS 20.0f
#define PERF_SIXTY_HERTZ_MILLISECONDS 16.667f
#define PERF_METRIC_ROW_HEIGHT 20.0f
#define PERF_STEP_BUTTON_WIDTH 62.0f

static void PerfDrawFrameGraph(fuiContext *ui, const PerfMetrics *metrics, const fuiRect rect) {
	fuiTheme *theme = fuiGetTheme(ui);
	fuiDrawRect(ui, rect, theme->widgetTrackColor);
	fuiDrawRectOutline(ui, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	// Scaled to the tallest sample but never below a floor, so a quiet graph is a flat line near the
	// bottom rather than noise blown up to the full height.
	float tallestSample = PERF_GRAPH_FLOOR_MILLISECONDS;
	for(int32_t sampleIndex = 0; sampleIndex < PERF_HISTORY_COUNT; ++sampleIndex) {
		float sample = metrics->frameHistory[sampleIndex];
		if(sample > tallestSample) {
			tallestSample = sample;
		}
	}

	fuiColor withinBudgetColor = fuiColorRGBA(0.35f, 0.75f, 0.45f, 1.0f);
	fuiColor overBudgetColor = fuiColorRGBA(0.85f, 0.42f, 0.30f, 1.0f);
	float barWidth = rect.w / (float)PERF_HISTORY_COUNT;
	for(int32_t sampleIndex = 0; sampleIndex < PERF_HISTORY_COUNT; ++sampleIndex) {
		// Read oldest first, so the newest sample is at the right hand edge and the graph reads forward.
		int32_t ringIndex = (metrics->historyWriteIndex + sampleIndex) % PERF_HISTORY_COUNT;
		float sample = metrics->frameHistory[ringIndex];
		if(sample <= 0.0f) {
			continue;
		}
		float normalizedHeight = sample / tallestSample;
		float barHeight = normalizedHeight * rect.h;
		float barLeft = rect.x + (float)sampleIndex * barWidth;
		float barTop = rect.y + rect.h - barHeight;
		fuiRect bar = fuiRectMake(barLeft, barTop, barWidth, barHeight);
		bool isOverBudget = (sample > PERF_SIXTY_HERTZ_MILLISECONDS);
		fuiDrawRect(ui, bar, isOverBudget ? overBudgetColor : withinBudgetColor);
	}

	// The sixty hertz budget, so a bar crossing it is visible without reading the number next to it.
	float budgetFraction = PERF_SIXTY_HERTZ_MILLISECONDS / tallestSample;
	if(budgetFraction < 1.0f) {
		float lineY = rect.y + rect.h - budgetFraction * rect.h;
		fuiVec2 lineStart = fuiV2(rect.x, lineY);
		fuiVec2 lineEnd = fuiV2(rect.x + rect.w, lineY);
		fuiColor budgetLineColor = fuiColorRGBA(0.95f, 0.85f, 0.35f, 0.7f);
		fuiDrawLine(ui, lineStart, lineEnd, budgetLineColor, 1.0f);
	}
}

//! One "name  value" line of the readout. Two columns would need a stack per row for no gain at this width
static void PerfMetricLine(fuiContext *ui, const char *name, const char *value) {
	char line[96];
	fplStringFormat(line, fplArrayCount(line), "%-13s %s", name, value);
	fuiRect lineRect = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
	fuiLabel(ui, lineRect, line);
}

//! A row of step buttons that all set the same request, which is the shape both scale rows below take
static void PerfStepButtonRow(fuiContext *ui, const char *stackId, const char *const *labels, const int32_t labelCount, const int32_t currentIndex, int32_t *requestedIndex) {
	fuiRect rowRect = fuiLayoutSlot(ui, PERF_ROW_HEIGHT);
	fuiBeginStackAt(ui, stackId, FUI_AXIS_HORIZONTAL, rowRect, FUI_SPACING_FROM_THEME);
	for(int32_t labelIndex = 0; labelIndex < labelCount; ++labelIndex) {
		fuiRect buttonRect = fuiLayoutSlot(ui, PERF_STEP_BUTTON_WIDTH);
		bool isTheCurrentStep = (currentIndex == labelIndex);
		// The step already showing is drawn as disabled, which is the cheapest way to say "you are here"
		// without a second widget kind in the row.
		bool wasClicked = fuiButtonEx(ui, buttonRect, labels[labelIndex], !isTheCurrentStep);
		if(wasClicked) {
			*requestedIndex = labelIndex;
		}
	}
	fuiEndStack(ui);
}

static void PerfBuildMetricsPanel(fuiContext *ui, PerfState *state) {
	if(!state->showMetricsPanel) {
		return;
	}

	const float panelTakesTheWholeHeight = 0.0f;
	bool panelIsOpen = fuiBeginPanel(ui, "Metrics", FUI_DOCK_LEFT, 0.0f, 0.0f, PERF_METRICS_PANEL_WIDTH, panelTakesTheWholeHeight);
	if(panelIsOpen) {
		const PerfMetrics *metrics = &state->metrics;
		const PerfDataSet *data = &state->data;
		char value[64];

		fuiRect timingsCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiLabel(ui, timingsCaption, "-- per frame --");

		fplStringFormat(value, fplArrayCount(value), "%7.3f ms", metrics->smoothedBuildMilliseconds);
		PerfMetricLine(ui, "Build", value);
		fplStringFormat(value, fplArrayCount(value), "%7.3f ms", metrics->smoothedRenderMilliseconds);
		PerfMetricLine(ui, "Render", value);
		fplStringFormat(value, fplArrayCount(value), "%7.3f ms", metrics->smoothedFrameMilliseconds);
		PerfMetricLine(ui, "Frame", value);
		fplStringFormat(value, fplArrayCount(value), "%7.3f ms", metrics->worstFrameMilliseconds);
		PerfMetricLine(ui, "Worst frame", value);

		double framesPerSecond = 0.0;
		if(metrics->smoothedFrameMilliseconds > 0.0) {
			framesPerSecond = PERF_MILLISECONDS_PER_SECOND / metrics->smoothedFrameMilliseconds;
		}
		fplStringFormat(value, fplArrayCount(value), "%7.1f", framesPerSecond);
		PerfMetricLine(ui, "Frames/sec", value);

		fuiRect firstSeparator = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiSeparator(ui, firstSeparator);

		fuiRect drawCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiLabel(ui, drawCaption, "-- draw data --");

		fplStringFormat(value, fplArrayCount(value), "%7u", metrics->commandCount);
		PerfMetricLine(ui, "Commands", value);
		fplStringFormat(value, fplArrayCount(value), "%7u", metrics->vertexCount);
		PerfMetricLine(ui, "Vertices", value);
		fplStringFormat(value, fplArrayCount(value), "%7u", metrics->indexCount);
		PerfMetricLine(ui, "Indices", value);
		fplStringFormat(value, fplArrayCount(value), "%7u", metrics->textByteCount);
		PerfMetricLine(ui, "Text bytes", value);
		fplStringFormat(value, fplArrayCount(value), "%7u", metrics->menuRowCount);
		PerfMetricLine(ui, "Menu rows", value);

		const double bytesPerKilobyte = 1024.0;
		double arenaKilobytes = (double)metrics->arenaByteCount / bytesPerKilobyte;
		fplStringFormat(value, fplArrayCount(value), "%7.0f KB", arenaKilobytes);
		PerfMetricLine(ui, "UI arena", value);

		fuiRect secondSeparator = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiSeparator(ui, secondSeparator);

		fuiRect dataCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiLabel(ui, dataCaption, "-- dataset --");

		fplStringFormat(value, fplArrayCount(value), "%7d", data->tableRowCount);
		PerfMetricLine(ui, "Table rows", value);
		fplStringFormat(value, fplArrayCount(value), "%7d", data->listItemCount);
		PerfMetricLine(ui, "List rows", value);
		fplStringFormat(value, fplArrayCount(value), "%7d", data->textLineCount);
		PerfMetricLine(ui, "Text lines", value);
		fplStringFormat(value, fplArrayCount(value), "%7d", data->menuNodeCount);
		PerfMetricLine(ui, "Menu nodes", value);

		const double bytesPerMegabyte = 1024.0 * 1024.0;
		double dataMegabytes = (double)data->generatedByteCount / bytesPerMegabyte;
		fplStringFormat(value, fplArrayCount(value), "%7.1f MB", dataMegabytes);
		PerfMetricLine(ui, "Data size", value);
		fplStringFormat(value, fplArrayCount(value), "%7.0f ms", data->generationMilliseconds);
		PerfMetricLine(ui, "Generated in", value);

		fuiRect textStepCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiLabel(ui, textStepCaption, "Text lines");
		PerfStepButtonRow(ui, "textsteps", g_perfTextLabels, PERF_TEXT_STEP_COUNT, state->textStepIndex, &state->requestedTextStepIndex);

		fuiRect menuStepCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
		fuiLabel(ui, menuStepCaption, "Items per menu");
		PerfStepButtonRow(ui, "menusteps", g_perfMenuLabels, PERF_MENU_STEP_COUNT, state->menuStepIndex, &state->requestedMenuStepIndex);

		if(state->showGraph) {
			fuiRect graphCaption = fuiLayoutSlot(ui, PERF_METRIC_ROW_HEIGHT);
			fuiLabel(ui, graphCaption, "-- frame time --");
			fuiRect graphRect = fuiLayoutSlot(ui, PERF_GRAPH_HEIGHT);
			PerfDrawFrameGraph(ui, metrics, graphRect);
		}
	}
	fuiEndPanel(ui);
}

// ----------------------------------------------------------------------------
// The data widgets
//
// One per tab, so a reading can be taken of ONE widget kind. The last tab builds all three at once,
// which is what an actual application window looks like and what the totals have to survive.
// ----------------------------------------------------------------------------

#define PERF_TABLE_ID "perftable"
#define PERF_LIST_ID "perflist"
#define PERF_TEXT_ID "perftext"
#define PERF_NOTE_HEIGHT 22.0f

static void PerfBuildListViewTab(fuiContext *ui, PerfState *state, const fuiRect rect) {
	// A list longer than FUI_MAX_SORTABLE_ROWS is not sorted at all, and the library says nothing about
	// it: the header still takes the click and still draws its arrow, and the rows simply stay in the
	// caller's own order. Saying so here is the difference between a limit and a mystery.
	bool isBeyondTheSortCap = (state->data.tableRowCount > (int32_t)FUI_MAX_SORTABLE_ROWS);
	float noteHeight = isBeyondTheSortCap ? PERF_NOTE_HEIGHT : 0.0f;
	fuiRect listRect = fuiRectMake(rect.x, rect.y, rect.w, rect.h - noteHeight);

	fuiListViewSetSortable(ui, PERF_TABLE_ID, state->sortIsEnabled);

	bool wasActivated = false;
	bool selectionChanged = fuiListViewEx(ui, listRect, PERF_TABLE_ID, g_perfTableColumns, PERF_TABLE_COLUMN_COUNT, state->data.tableCells, state->data.tableRowCount, &state->tableSelection, fpl_null, &wasActivated);
	if(selectionChanged) {
		size_t nameCellIndex = (size_t)state->tableSelection * (size_t)PERF_TABLE_COLUMN_COUNT + (size_t)PERF_COLUMN_NAME;
		const char *pickedName = state->data.tableCells[nameCellIndex];
		char message[PERF_STATUS_TEXT_MAX];
		fplStringFormat(message, fplArrayCount(message), "Row %d of %d picked: %s", state->tableSelection + 1, state->data.tableRowCount, pickedName);
		PerfSay(state, message);
	}

	if(isBeyondTheSortCap) {
		char note[PERF_STATUS_TEXT_MAX];
		fplStringFormat(note, fplArrayCount(note), "%d rows is past FUI_MAX_SORTABLE_ROWS (%d) - a header click sorts nothing here", state->data.tableRowCount, (int32_t)FUI_MAX_SORTABLE_ROWS);
		fuiRect noteRect = fuiRectMake(rect.x, rect.y + rect.h - noteHeight, rect.w, noteHeight);
		fuiLabel(ui, noteRect, note);
	}
}

static void PerfBuildListBoxTab(fuiContext *ui, PerfState *state, const fuiRect rect) {
	bool selectionChanged = fuiListBox(ui, rect, PERF_LIST_ID, state->data.listItems, state->data.listItemCount, &state->listSelection);
	if(selectionChanged) {
		char message[PERF_STATUS_TEXT_MAX];
		fplStringFormat(message, fplArrayCount(message), "List row %d of %d picked", state->listSelection + 1, state->data.listItemCount);
		PerfSay(state, message);
	}
}

static void PerfBuildTextBoxTab(fuiContext *ui, PerfState *state, const fuiRect rect) {
	// A multiline field breaks its whole buffer into lines and then shows the first FUI_MAX_TEXT_LINES of
	// them, with no way to scroll to the rest. At five hundred lines that is invisible; at two hundred
	// thousand it is the whole story, so the note says what the box is really showing.
	bool isBeyondTheLineCap = (state->data.textLineCount > (int32_t)FUI_MAX_TEXT_LINES);
	float noteHeight = isBeyondTheLineCap ? PERF_NOTE_HEIGHT : 0.0f;
	fuiRect fieldRect = fuiRectMake(rect.x, rect.y, rect.w, rect.h - noteHeight);

	const bool isMultiline = true;
	int32_t capacity = (int32_t)state->data.textCapacity;
	(void)fuiTextInputEx(ui, fieldRect, PERF_TEXT_ID, state->data.textBuffer, capacity, isMultiline, state->wordWrapIsOn);

	if(isBeyondTheLineCap) {
		char note[PERF_STATUS_TEXT_MAX];
		fplStringFormat(note, fplArrayCount(note), "%d lines loaded, %d shown - FUI_MAX_TEXT_LINES caps it and the field does not scroll", state->data.textLineCount, (int32_t)FUI_MAX_TEXT_LINES);
		fuiRect noteRect = fuiRectMake(rect.x, rect.y + rect.h - noteHeight, rect.w, noteHeight);
		fuiLabel(ui, noteRect, note);
	}
}

#define PERF_SPLIT_FRACTION 0.55f

static void PerfBuildEverythingTab(fuiContext *ui, PerfState *state, const fuiRect rect) {
	float leftWidth = rect.w * PERF_SPLIT_FRACTION;
	float rightWidth = rect.w - leftWidth - PERF_CONTENT_INSET;
	float rightLeft = rect.x + leftWidth + PERF_CONTENT_INSET;
	float halfHeight = (rect.h - PERF_CONTENT_INSET) * 0.5f;

	fuiRect tableRect = fuiRectMake(rect.x, rect.y, leftWidth, rect.h);
	fuiRect listRect = fuiRectMake(rightLeft, rect.y, rightWidth, halfHeight);
	fuiRect textRect = fuiRectMake(rightLeft, rect.y + halfHeight + PERF_CONTENT_INSET, rightWidth, halfHeight);

	PerfBuildListViewTab(ui, state, tableRect);
	PerfBuildListBoxTab(ui, state, listRect);
	PerfBuildTextBoxTab(ui, state, textRect);
}

static void PerfBuildContent(fuiContext *ui, PerfState *state, const fuiRect rect) {
	fuiRect tabStripRect = fuiRectMake(rect.x, rect.y, rect.w, PERF_TAB_STRIP_HEIGHT);
	int32_t tabCount = (int32_t)fplArrayCount(g_perfTabNames);
	state->activeTab = fuiTabControl(ui, tabStripRect, "contenttabs", g_perfTabNames, tabCount);

	float contentTop = rect.y + PERF_TAB_STRIP_HEIGHT + PERF_CONTENT_INSET;
	float contentHeight = rect.y + rect.h - contentTop;
	fuiRect contentRect = fuiRectMake(rect.x, contentTop, rect.w, contentHeight);

	switch(state->activeTab) {
		case PERF_TAB_LIST_BOX:
			PerfBuildListBoxTab(ui, state, contentRect);
			break;
		case PERF_TAB_TEXT_BOX:
			PerfBuildTextBoxTab(ui, state, contentRect);
			break;
		case PERF_TAB_EVERYTHING:
			PerfBuildEverythingTab(ui, state, contentRect);
			break;
		case PERF_TAB_LIST_VIEW:
		default:
			PerfBuildListViewTab(ui, state, contentRect);
			break;
	}
}

#define PERF_CONTEXT_MENU_ID "perfcontextmenu"

//! The first generated menu's children, straight into a context menu. One popup, every row of it emitted,
//! which is the worst case a menu can put on a frame and the easiest one to open on purpose
static void PerfBuildContextMenu(fuiContext *ui, PerfState *state) {
	bool isOpen = fuiBeginContextMenu(ui, PERF_CONTEXT_MENU_ID);
	if(isOpen && state->data.menuTopLevelCount > 0) {
		const PerfMenuNode *menu = &state->data.menuTopLevel[0];
		const int32_t popupDepth = 1;
		for(int32_t childIndex = 0; childIndex < menu->childCount; ++childIndex) {
			PerfBuildMenuNode(ui, state, &menu->children[childIndex], popupDepth);
		}
	}
	fuiEndContextMenu(ui);
}

static void PerfBuildStatusBar(fuiContext *ui, PerfState *state, const fuiRect statusRect) {
	fuiBeginStatusBar(ui, "statusbar", statusRect);
	fuiStatusText(ui, state->statusMessage);

	const PerfMetrics *metrics = &state->metrics;
	char rightText[80];

	double framesPerSecond = 0.0;
	if(metrics->smoothedFrameMilliseconds > 0.0) {
		framesPerSecond = PERF_MILLISECONDS_PER_SECOND / metrics->smoothedFrameMilliseconds;
	}
	fplStringFormat(rightText, fplArrayCount(rightText), "%.0f fps", framesPerSecond);
	fuiStatusTextRight(ui, rightText);

	fplStringFormat(rightText, fplArrayCount(rightText), "build %.2f ms", metrics->smoothedBuildMilliseconds);
	fuiStatusTextRight(ui, rightText);

	fplStringFormat(rightText, fplArrayCount(rightText), "render %.2f ms", metrics->smoothedRenderMilliseconds);
	fuiStatusTextRight(ui, rightText);

	fplStringFormat(rightText, fplArrayCount(rightText), "%u cmds", metrics->commandCount);
	fuiStatusTextRight(ui, rightText);

	fplStringFormat(rightText, fplArrayCount(rightText), "%u menu rows", metrics->menuRowCount);
	fuiStatusTextRight(ui, rightText);

	fuiEndStatusBar(ui);
}

static void PerfBuildUserInterface(fuiContext *ui, PerfState *state, const bool rightWasPressed) {
	state->menuRowsThisFrame = 0u;

	// A right press the interface did not want opens the big popup wherever the cursor is. Asked of the
	// PREVIOUS frame, because nothing has been built yet and fuiWantsMouse would answer for a stale layout.
	if(rightWasPressed && !state->uiOwnedTheMouseLastFrame) {
		fuiOpenContextMenu(ui, PERF_CONTEXT_MENU_ID);
	}

	// The three bars come off the root container before anything else, so their thickness is the theme's
	// and their width is whatever the window is.
	float menuBarHeight = fuiMenuBarHeight(ui);
	float toolStripThickness = fuiToolStripThickness(ui);
	float statusBarHeight = fuiStatusBarHeight(ui);
	fuiRect menuBarRect = fuiLayoutDock(ui, FUI_DOCK_TOP, menuBarHeight);
	fuiRect toolStripRect = fuiLayoutDock(ui, FUI_DOCK_TOP, toolStripThickness);
	fuiRect statusBarRect = fuiLayoutDock(ui, FUI_DOCK_BOTTOM, statusBarHeight);

	PerfBuildToolStrip(ui, state, toolStripRect);
	PerfBuildMetricsPanel(ui, state);

	fuiRect contentRect = fuiLayoutRemaining(ui);
	fuiRect insetContentRect = fuiRectMake(contentRect.x + PERF_CONTENT_INSET, contentRect.y + PERF_CONTENT_INSET, contentRect.w - PERF_CONTENT_INSET * 2.0f, contentRect.h - PERF_CONTENT_INSET * 2.0f);
	PerfBuildContent(ui, state, insetContentRect);

	PerfBuildStatusBar(ui, state, statusBarRect);

	// Built LAST, because a popup floats above the docked layout and a later call takes the cursor from an
	// earlier one with no z ordering anywhere.
	PerfBuildMenuBar(ui, state, menuBarRect);
	PerfBuildContextMenu(ui, state);
}

// ----------------------------------------------------------------------------
// Headless benchmark
//
// The whole point of the window is to SEE the cost. The point of this is to be able to compare two
// versions of final_ui.h without one, from a terminal, with numbers that do not move between runs.
//
// No window and no OpenGL: the font is baked on the processor, the atlas is never uploaded, and the
// texture identifier the draw commands carry is simply zero. Everything the library does to build a
// frame - layout, hit testing, sorting, text measuring, tessellation - happens exactly as it does with
// a window in front of it. Only the driver is missing, and the command count stands in for it.
// ----------------------------------------------------------------------------

#define PERF_BENCHMARK_WARMUP_FRAMES 8
#define PERF_BENCHMARK_SAMPLE_FRAMES 41
#define PERF_BENCHMARK_WIDTH 1600
#define PERF_BENCHMARK_HEIGHT 940
#define PERF_BENCHMARK_MARGIN 20.0f
#define PERF_BENCHMARK_MENU_ANCHOR 40.0f

typedef enum PerfSubject {
	PerfSubject_ListView = 0,
	PerfSubject_ListViewSorted,
	PerfSubject_ListViewResorted,
	PerfSubject_ListBox,
	PerfSubject_TextBox,
	PerfSubject_MenuPopup,
	PerfSubject_Everything,
} PerfSubject;

typedef struct PerfCase {
	const char *name;
	PerfSubject subject;
	int32_t scaleStepIndex;
	int32_t textStepIndex;
	int32_t menuStepIndex;
} PerfCase;

// Every case names the ONE thing it varies. Reading down a column of these is the whole experiment: the
// scale goes up by ten and the build time either follows it or it does not.
static const PerfCase g_perfCases[] = {
	{ "listview 1K",          PerfSubject_ListView,       0, 0, 0 },
	{ "listview 10K",         PerfSubject_ListView,       1, 0, 0 },
	{ "listview 100K",        PerfSubject_ListView,       2, 0, 0 },
	{ "listview 1M",          PerfSubject_ListView,       4, 0, 0 },
	{ "listview sorted 1K",   PerfSubject_ListViewSorted, 0, 0, 0 },
	{ "listview sorted 10K",  PerfSubject_ListViewSorted, 1, 0, 0 },
	{ "listview sorted 100K", PerfSubject_ListViewSorted, 2, 0, 0 },
	{ "listview sorted 1M",   PerfSubject_ListViewSorted, 4, 0, 0 },
	{ "listview resort 1K",   PerfSubject_ListViewResorted, 0, 0, 0 },
	{ "listview resort 10K",  PerfSubject_ListViewResorted, 1, 0, 0 },
	{ "listview resort 100K", PerfSubject_ListViewResorted, 2, 0, 0 },
	{ "listbox 500",          PerfSubject_ListBox,        0, 0, 0 },
	{ "listbox 5K",           PerfSubject_ListBox,        1, 0, 0 },
	{ "listbox 50K",          PerfSubject_ListBox,        2, 0, 0 },
	{ "listbox 500K",         PerfSubject_ListBox,        4, 0, 0 },
	{ "textbox 500",          PerfSubject_TextBox,        0, 0, 0 },
	{ "textbox 5K",           PerfSubject_TextBox,        0, 1, 0 },
	{ "textbox 50K",          PerfSubject_TextBox,        0, 2, 0 },
	{ "textbox 200K",         PerfSubject_TextBox,        0, 3, 0 },
	{ "menu 10 rows",         PerfSubject_MenuPopup,      0, 0, 0 },
	{ "menu 40 rows",         PerfSubject_MenuPopup,      0, 0, 1 },
	{ "menu 120 rows",        PerfSubject_MenuPopup,      0, 0, 2 },
	{ "menu 400 rows",        PerfSubject_MenuPopup,      0, 0, 3 },
	{ "everything 10K",       PerfSubject_Everything,     1, 1, 1 },
	{ "everything 100K",      PerfSubject_Everything,     2, 2, 2 },
};

static void PerfBuildBenchmarkFrame(fuiContext *ui, PerfState *state, const PerfSubject subject, const fuiRect rect, const bool isTheFirstFrame) {
	state->menuRowsThisFrame = 0u;
	switch(subject) {
		case PerfSubject_ListViewSorted:
		{
			// Seeded rather than clicked, because there is no cursor to click a header with. It takes only
			// the first time it is asked, so calling it every frame is what the library expects.
			fuiListViewSetSortDefault(ui, PERF_TABLE_ID, PERF_COLUMN_NAME, true);
			PerfBuildListViewTab(ui, state, rect);
		} break;

		case PerfSubject_ListViewResorted:
		{
			// The same list, with the cached order thrown away every single frame. Nothing real does this -
			// it is what the sorted case would cost if the order were not kept, which is the number the
			// cache has to be judged against.
			fuiListViewSetSortDefault(ui, PERF_TABLE_ID, PERF_COLUMN_NAME, true);
			fuiListViewInvalidateSort(ui, PERF_TABLE_ID);
			PerfBuildListViewTab(ui, state, rect);
		} break;

		case PerfSubject_ListBox:
			PerfBuildListBoxTab(ui, state, rect);
			break;

		case PerfSubject_TextBox:
			PerfBuildTextBoxTab(ui, state, rect);
			break;

		case PerfSubject_MenuPopup:
		{
			if(isTheFirstFrame) {
				fuiOpenContextMenu(ui, PERF_CONTEXT_MENU_ID);
			}
			PerfBuildContextMenu(ui, state);
		} break;

		case PerfSubject_Everything:
			PerfBuildEverythingTab(ui, state, rect);
			break;

		case PerfSubject_ListView:
		default:
			PerfBuildListViewTab(ui, state, rect);
			break;
	}
}

//! Sorts the samples in place so the middle one can be taken. Insertion sort, because the array is tiny
static void PerfSortSamples(double *samples, const int32_t sampleCount) {
	for(int32_t sampleIndex = 1; sampleIndex < sampleCount; ++sampleIndex) {
		double sample = samples[sampleIndex];
		int32_t insertIndex = sampleIndex - 1;
		while(insertIndex >= 0 && samples[insertIndex] > sample) {
			samples[insertIndex + 1] = samples[insertIndex];
			insertIndex -= 1;
		}
		samples[insertIndex + 1] = sample;
	}
}

static void PerfRunBenchmarkCase(fuiContext *ui, PerfState *state, const PerfCase *benchmarkCase) {
	state->requestedScaleStepIndex = benchmarkCase->scaleStepIndex;
	state->requestedTextStepIndex = benchmarkCase->textStepIndex;
	state->requestedMenuStepIndex = benchmarkCase->menuStepIndex;
	PerfApplyRequestedScale(state);
	if(!state->data.isComplete) {
		printf("%-22s  out of memory\n", benchmarkCase->name);
		return;
	}

	fuiRect contentRect = fuiRectMake(PERF_BENCHMARK_MARGIN, PERF_BENCHMARK_MARGIN, (float)PERF_BENCHMARK_WIDTH - PERF_BENCHMARK_MARGIN * 2.0f, (float)PERF_BENCHMARK_HEIGHT - PERF_BENCHMARK_MARGIN * 2.0f);

	double samples[PERF_BENCHMARK_SAMPLE_FRAMES];
	uint32_t lastCommandCount = 0;
	uint32_t lastVertexCount = 0;
	uint32_t lastMenuRowCount = 0;
	int32_t totalFrames = PERF_BENCHMARK_WARMUP_FRAMES + PERF_BENCHMARK_SAMPLE_FRAMES;
	const float sixtyHertzDelta = 1.0f / 60.0f;

	for(int32_t frameIndex = 0; frameIndex < totalFrames; ++frameIndex) {
		bool isTheFirstFrame = (frameIndex == 0);

		fuiInput input = fuiZeroInput();
		input.windowSize = fuiV2i(PERF_BENCHMARK_WIDTH, PERF_BENCHMARK_HEIGHT);
		input.deltaTime = sixtyHertzDelta;
		input.isActive = true;
		// The popup is anchored where the cursor was when it opened, so the first frame puts the cursor
		// somewhere sane and every frame after moves it out of the popup - a cursor resting on a submenu
		// row would open that submenu too and the case would stop measuring one popup.
		if(isTheFirstFrame) {
			input.mousePosition = fuiV2(PERF_BENCHMARK_MENU_ANCHOR, PERF_BENCHMARK_MENU_ANCHOR);
		} else {
			input.mousePosition = fuiV2((float)PERF_BENCHMARK_WIDTH - PERF_BENCHMARK_MARGIN, (float)PERF_BENCHMARK_HEIGHT - PERF_BENCHMARK_MARGIN);
		}

		fplTimestamp buildStart = fplTimestampQuery();
		fuiBeginFrame(ui, &input, FUI_PASS_BOTH);
		PerfBuildBenchmarkFrame(ui, state, benchmarkCase->subject, contentRect, isTheFirstFrame);
		fuiEndFrame(ui);
		fplTimestamp buildEnd = fplTimestampQuery();

		const fuiDrawData *drawData = fuiGetDrawData(ui);
		lastCommandCount = drawData->commandCount;
		lastVertexCount = drawData->vertexCount;
		lastMenuRowCount = state->menuRowsThisFrame;

		if(frameIndex >= PERF_BENCHMARK_WARMUP_FRAMES) {
			double elapsedSeconds = fplTimestampElapsed(buildStart, buildEnd);
			samples[frameIndex - PERF_BENCHMARK_WARMUP_FRAMES] = elapsedSeconds * PERF_MILLISECONDS_PER_SECOND;
		}
	}

	PerfSortSamples(samples, PERF_BENCHMARK_SAMPLE_FRAMES);
	double fastest = samples[0];
	double median = samples[PERF_BENCHMARK_SAMPLE_FRAMES / 2];
	double slowest = samples[PERF_BENCHMARK_SAMPLE_FRAMES - 1];

	printf("%-22s %9.3f %9.3f %9.3f %9u %9u %9u\n", benchmarkCase->name, median, fastest, slowest, lastCommandCount, lastVertexCount, lastMenuRowCount);
	fflush(stdout);
}

static int PerfRunBenchmark(void) {
	if(!fplPlatformInit(fplInitFlags_None, fpl_null)) {
		fprintf(stderr, "failed to initialize the platform\n");
		return 1;
	}

	fuiStbttFont bakedFont;
	fuiStbttBakeSettings bakeSettings = fuiStbttDefaultBakeSettings();
	bakeSettings.pixelHeight = PERF_FONT_PIXEL_HEIGHT;
	bakeSettings.atlasWidth = PERF_FONT_ATLAS_SIDE;
	bakeSettings.atlasHeight = PERF_FONT_ATLAS_SIDE;
	if(!fuiStbttFontBake(&bakedFont, ptr_fontBitstreamVeraRegular, &bakeSettings)) {
		fprintf(stderr, "failed to bake the font\n");
		fplPlatformRelease();
		return 1;
	}

	// No atlas is uploaded, so the commands carry a texture nobody will ever bind. Nothing in the library
	// dereferences it, and the geometry it builds is the same either way.
	const fuiTextureId noAtlasTexture = 0;
	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, noAtlasTexture);

	fuiContext ui;
	if(!fuiInit(&ui, &font, fpl_null)) {
		fprintf(stderr, "failed to initialize the user interface\n");
		fuiStbttFontRelease(&bakedFont);
		fplPlatformRelease();
		return 1;
	}

	fuiTheme *theme = fuiGetTheme(&ui);
	const float denseFontHeight = 13.0f;
	const float denseRowHeight = 20.0f;
	theme->fontHeight = denseFontHeight;
	theme->menuItemFontHeight = denseFontHeight;
	theme->menuItemHeight = denseRowHeight;

	PerfState state;
	PerfInit(&state);

	printf("final_ui.h build cost, %d warmup frames then %d measured, one widget per case\n\n", (int)PERF_BENCHMARK_WARMUP_FRAMES, (int)PERF_BENCHMARK_SAMPLE_FRAMES);
	printf("%-22s %9s %9s %9s %9s %9s %9s\n", "case", "median", "fastest", "slowest", "commands", "vertices", "menurows");
	printf("%-22s %9s %9s %9s %9s %9s %9s\n", "----------------------", "---------", "---------", "---------", "---------", "---------", "---------");

	int32_t caseCount = (int32_t)fplArrayCount(g_perfCases);
	for(int32_t caseIndex = 0; caseIndex < caseCount; ++caseIndex) {
		const PerfCase *benchmarkCase = &g_perfCases[caseIndex];
		PerfRunBenchmarkCase(&ui, &state, benchmarkCase);
	}

	printf("\nmilliseconds per frame, arena %zu bytes\n", fuiGetAllocatedSize(&ui));

	PerfDataSetRelease(&state.data);
	fuiRelease(&ui);
	fuiStbttFontRelease(&bakedFont);
	fplPlatformRelease();
	return 0;
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

int main(int argc, char **argv) {
	for(int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
		bool isTheBenchmarkFlag = (strcmp(argv[argumentIndex], "--benchmark") == 0);
		if(isTheBenchmarkFlag) {
			int benchmarkResult = PerfRunBenchmark();
			return benchmarkResult;
		}
	}

	fplSettings settings = fplZeroInit;
	fplSetDefaultSettings(&settings);
	fplCopyString(PERF_WINDOW_TITLE, settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = PERF_WINDOW_WIDTH;
	settings.window.windowSize.height = PERF_WINDOW_HEIGHT;
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	// OFF on purpose: with vsync on, every frame under the budget reads as exactly one refresh and the
	// build time is the only number left that means anything. A workbench wants the whole curve.
	settings.video.isVSync = false;

	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_Video, &settings)) {
		fprintf(stderr, "failed to initialize the platform\n");
		return 1;
	}
	if(!fglLoadOpenGL(true)) {
		fprintf(stderr, "failed to load OpenGL\n");
		fplPlatformRelease();
		return 1;
	}

	fuiStbttFont bakedFont;
	fuiStbttBakeSettings bakeSettings = fuiStbttDefaultBakeSettings();
	bakeSettings.pixelHeight = PERF_FONT_PIXEL_HEIGHT;
	bakeSettings.atlasWidth = PERF_FONT_ATLAS_SIDE;
	bakeSettings.atlasHeight = PERF_FONT_ATLAS_SIDE;
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

	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = fuiFplGetClipboardText;
	platform.setClipboardText = fuiFplSetClipboardText;
	fuiSetPlatform(&ui, &platform);

	// Rows this dense want a smaller type than the default, or eight columns of a table do not fit in a
	// window at all and the demo measures wrapping instead of what it came to measure.
	fuiTheme *theme = fuiGetTheme(&ui);
	const float denseFontHeight = 13.0f;
	const float denseRowHeight = 20.0f;
	theme->fontHeight = denseFontHeight;
	theme->menuItemFontHeight = denseFontHeight;
	theme->menuItemHeight = denseRowHeight;

	PerfState state;
	PerfInit(&state);
	PerfApplyRequestedScale(&state);

	fuiFplInput bridge;
	fuiFplInputInit(&bridge);

	while(state.isRunning && fplWindowUpdate()) {
		fplTimestamp frameStart = fplTimestampQuery();

		fuiFplInputPumpEvents(&bridge);
		fuiFplInputBuild(&bridge);

		// Escape quits, but only when no dialog and no text field wants it first.
		bool keyboardIsTaken = fuiWantsKeyboard(&ui);
		bool escapeWentDown = fuiKeyWentDown(&ui, FUI_KEY_ESCAPE);
		if(escapeWentDown && !keyboardIsTaken) {
			state.isRunning = false;
		}

		fplTimestamp buildStart = fplTimestampQuery();
		fuiBeginFrame(&ui, &bridge.input, FUI_PASS_BOTH);
		PerfBuildUserInterface(&ui, &state, bridge.rightPressedThisFrame);
		fuiEndFrame(&ui);
		fplTimestamp buildEnd = fplTimestampQuery();

		state.uiOwnedTheMouseLastFrame = fuiWantsMouse(&ui);

		const fuiDrawData *drawData = fuiGetDrawData(&ui);

		glViewport(0, 0, bridge.input.windowSize.x, bridge.input.windowSize.y);
		glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		fplTimestamp renderStart = fplTimestampQuery();
		fuiGL1Render(drawData);
		// The backend hands the driver a draw call per command and returns before any of them have run, so
		// a finish is the only way to time the work rather than the queueing of it.
		glFinish();
		fplTimestamp renderEnd = fplTimestampQuery();

		fplVideoFlip();

		fplTimestamp frameEnd = fplTimestampQuery();

		double buildSeconds = fplTimestampElapsed(buildStart, buildEnd);
		double renderSeconds = fplTimestampElapsed(renderStart, renderEnd);
		double frameSeconds = fplTimestampElapsed(frameStart, frameEnd);
		PerfMetricsPush(&state.metrics, buildSeconds * PERF_MILLISECONDS_PER_SECOND, renderSeconds * PERF_MILLISECONDS_PER_SECOND, frameSeconds * PERF_MILLISECONDS_PER_SECOND);

		state.metrics.commandCount = drawData->commandCount;
		state.metrics.vertexCount = drawData->vertexCount;
		state.metrics.indexCount = drawData->indexCount;
		state.metrics.textByteCount = drawData->textBufferSize;
		state.metrics.arenaByteCount = fuiGetAllocatedSize(&ui);
		state.metrics.menuRowCount = state.menuRowsThisFrame;

		// BETWEEN frames, never during one: the list view holds the cell array for the whole of its call.
		PerfApplyRequestedScale(&state);
	}

	PerfDataSetRelease(&state.data);
	fuiRelease(&ui);
	fuiGL1DeleteTexture(atlasTexture);
	fuiStbttFontRelease(&bakedFont);
	fglUnloadOpenGL();
	fplPlatformRelease();
	return 0;
}
