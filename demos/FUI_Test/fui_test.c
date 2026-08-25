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

	The loop is ONE pass (FUI_PASS_BOTH), which is what a plain demo wants: build the interface once per
	frame and let hover resolve against the previous frame's layout. A host that has to know whether the
	interface wants the mouse before it decides what to do with the cursor - a game with a world behind
	the interface, say - builds twice instead, FUI_PASS_INTERACT then FUI_PASS_DRAW.

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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define DEMO_WINDOW_TITLE "final_ui.h demo (FPL + OpenGL)"
#define DEMO_WINDOW_WIDTH 1180
#define DEMO_WINDOW_HEIGHT 760

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

// The icon sheet the demo draws itself: four square cells in a row, one channel of coverage, uploaded the
// same way the font atlas is. No asset to ship, and a worked example of what fuiListIcons wants.
#define DEMO_ICON_CELL_SIDE 32u
#define DEMO_ICON_CELL_COUNT 4u
#define DEMO_ICON_SHEET_WIDTH (DEMO_ICON_CELL_SIDE * DEMO_ICON_CELL_COUNT)
#define DEMO_ICON_SHEET_HEIGHT DEMO_ICON_CELL_SIDE

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

	char statusMessage[DEMO_STATUS_MESSAGE_MAX];
	float framesPerSecond;

	/*
		Whether the interface owned the cursor at the END of the previous frame.

		In one pass mode fuiWantsMouse is only final once the frame is finished: it becomes true as the
		widget under the cursor is built, and an open menu forces it in fuiEndFrame. Anything asking the
		question WHILE the interface is being built - the world deciding whether a click was meant for it,
		a status line reporting the answer - therefore has to ask about the previous frame.

		A host that cannot live with that one frame of lag builds twice instead, FUI_PASS_INTERACT before it
		reads input and FUI_PASS_DRAW when it renders, and then the answer is current inside the same frame.
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
	The icon sheet, drawn by the demo into one channel of coverage - a square, a triangle, a ring and a
	diamond, one per kind. It goes up through the same call the font atlas does, because to this backend an
	atlas IS a coverage bitmap, and a list icon and a glyph ask exactly the same thing of it.

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

	memset(coveragePixels, 0, (size_t)sheetWidth * (size_t)cellSide);
	for(int32_t cellIndex = 0; cellIndex < (int32_t)DEMO_ICON_CELL_COUNT; ++cellIndex) {
		for(int32_t y = 0; y < cellSide; ++y) {
			for(int32_t x = 0; x < cellSide; ++x) {
				float offsetX = ((float)x + 0.5f) - centre;
				float offsetY = ((float)y + 0.5f) - centre;
				float distanceSquared = offsetX * offsetX + offsetY * offsetY;
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
					default: // a diamond
						isInk = ((offsetX < 0.0f ? -offsetX : offsetX) + (offsetY < 0.0f ? -offsetY : offsetY)) <= outerRadius;
						break;
				}
				if(isInk) {
					int32_t sheetX = cellIndex * cellSide + x;
					coveragePixels[(size_t)y * (size_t)sheetWidth + (size_t)sheetX] = 255;
				}
			}
		}
	}
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
	demo->tableSelection = -1;
	demo->tablePlayingRow = -1;
	DemoBuildTable(demo);
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
	{ DEMO_COMMAND_NEW, "New", { FUI_KEY_N, (uint32_t)FUI_MOD_CONTROL }, fpl_null, fpl_null, DemoInvokeNew },
	{ DEMO_COMMAND_SAVE, "Save", { FUI_KEY_S, (uint32_t)FUI_MOD_CONTROL }, DemoSaveIsEnabled, fpl_null, DemoInvokeSave },
	{ DEMO_COMMAND_QUIT, "Quit", { FUI_KEY_Q, (uint32_t)FUI_MOD_CONTROL }, fpl_null, fpl_null, DemoInvokeQuit },
};

static const fuiCommandTable g_demoCommandTable = { g_demoCommandRows, (uint32_t)fplArrayCount(g_demoCommandRows) };

// ----------------------------------------------------------------------------
// FPL -> final_ui.h
//
// The library defines its own key enum rather than borrowing a platform's, so this table is the whole
// bridge. Everything else is a copy.
// ----------------------------------------------------------------------------

static fuiKey MapFplKeyToFuiKey(const fplKey key) {
	if(key >= fplKey_A && key <= fplKey_Z) {
		return (fuiKey)((int)FUI_KEY_A + ((int)key - (int)fplKey_A));
	}
	if(key >= fplKey_0 && key <= fplKey_9) {
		return (fuiKey)((int)FUI_KEY_0 + ((int)key - (int)fplKey_0));
	}
	if(key >= fplKey_F1 && key <= fplKey_F12) {
		return (fuiKey)((int)FUI_KEY_F1 + ((int)key - (int)fplKey_F1));
	}
	switch(key) {
		case fplKey_Backspace: return FUI_KEY_BACKSPACE;
		case fplKey_Tab: return FUI_KEY_TAB;
		case fplKey_Return: return FUI_KEY_RETURN;
		case fplKey_Escape: return FUI_KEY_ESCAPE;
		case fplKey_Space: return FUI_KEY_SPACE;
		case fplKey_PageUp: return FUI_KEY_PAGE_UP;
		case fplKey_PageDown: return FUI_KEY_PAGE_DOWN;
		case fplKey_End: return FUI_KEY_END;
		case fplKey_Home: return FUI_KEY_HOME;
		case fplKey_Left: return FUI_KEY_LEFT;
		case fplKey_Up: return FUI_KEY_UP;
		case fplKey_Right: return FUI_KEY_RIGHT;
		case fplKey_Down: return FUI_KEY_DOWN;
		case fplKey_Insert: return FUI_KEY_INSERT;
		case fplKey_Delete: return FUI_KEY_DELETE;
		case fplKey_LeftShift: return FUI_KEY_LEFT_SHIFT;
		case fplKey_RightShift: return FUI_KEY_RIGHT_SHIFT;
		case fplKey_LeftControl: return FUI_KEY_LEFT_CONTROL;
		case fplKey_RightControl: return FUI_KEY_RIGHT_CONTROL;
		case fplKey_LeftAlt: return FUI_KEY_LEFT_ALT;
		case fplKey_RightAlt: return FUI_KEY_RIGHT_ALT;
		// X11 reports the side, Win32 can report the generic one. Both have to arrive, or a shortcut works
		// on one platform and silently does not on the other.
		case fplKey_Shift: return FUI_KEY_LEFT_SHIFT;
		case fplKey_Control: return FUI_KEY_LEFT_CONTROL;
		case fplKey_Alt: return FUI_KEY_LEFT_ALT;
		default: return FUI_KEY_NONE;
	}
}

typedef struct InputBridge {
	fuiInput input;
	bool keyWasDown[FUI_KEY_COUNT];
	bool mouseWasDown[FUI_MOUSE_BUTTON_COUNT];
	bool windowHasFocus;
	float wheelThisFrame;
	uint32_t typedCodePoints[FUI_MAX_TEXT_INPUT];
	int32_t typedCount;
	bool rightPressedThisFrame;
	fplTimestamp lastTimestamp;
} InputBridge;

static void InputBridgeInit(InputBridge *bridge) {
	fplClearStruct(bridge);
	bridge->input = fuiZeroInput();
	bridge->windowHasFocus = true;
	bridge->lastTimestamp = fplTimestampQuery();
}

//! Drains the event queue for the things polling cannot give: typed characters, the wheel, and focus
static void InputBridgePumpEvents(InputBridge *bridge) {
	bridge->wheelThisFrame = 0.0f;
	bridge->typedCount = 0;
	bridge->rightPressedThisFrame = false;

	fplEvent event;
	while(fplPollEvent(&event)) {
		switch(event.type) {
			case fplEventType_Window:
			{
				if(event.window.type == fplWindowEventType_GotFocus) {
					bridge->windowHasFocus = true;
				} else if(event.window.type == fplWindowEventType_LostFocus) {
					bridge->windowHasFocus = false;
				}
			} break;

			case fplEventType_Keyboard:
			{
				if(event.keyboard.type == fplKeyboardEventType_Input) {
					// The FULL codepoint, not a byte. final_ui.h takes codepoints precisely so that a
					// platform layer narrowing this to a char cannot corrupt anything above U+00FF.
					uint32_t codePoint = (uint32_t)event.keyboard.keyCode;
					bool isPrintable = (codePoint >= 32u) && (codePoint != 127u);
					if(isPrintable && bridge->typedCount < (int32_t)FUI_MAX_TEXT_INPUT) {
						bridge->typedCodePoints[bridge->typedCount++] = codePoint;
					}
				}
			} break;

			case fplEventType_Mouse:
			{
				if(event.mouse.type == fplMouseEventType_Wheel) {
					bridge->wheelThisFrame += event.mouse.wheelDelta;
				} else if(event.mouse.type == fplMouseEventType_Button) {
					bool isRightPress = (event.mouse.mouseButton == fplMouseButtonType_Right) && (event.mouse.buttonState != fplButtonState_Release);
					if(isRightPress) {
						bridge->rightPressedThisFrame = true;
					}
				}
			} break;

			default:
				break;
		}
	}
}

/*
	Fills this frame's fuiInput.

	Held state comes from POLLING rather than from the event queue, which is the lesson the game learned
	the hard way: a key pressed and released inside one frame latches as stuck-down when its state is
	accumulated from events. The cost is that such a tap is not seen at all here - two half transitions in
	one frame is something only an event stream can express - and for an interface that is the right trade.
*/
static void InputBridgeBuild(InputBridge *bridge) {
	fplWindowSize windowSize = fplZeroInit;
	fplGetWindowSize(&windowSize);

	fplTimestamp now = fplTimestampQuery();
	double elapsedSeconds = fplTimestampElapsed(bridge->lastTimestamp, now);
	bridge->lastTimestamp = now;

	fuiInput *input = &bridge->input;
	input->windowSize = fuiV2i((int32_t)windowSize.width, (int32_t)windowSize.height);
	input->deltaTime = (float)elapsedSeconds;
	input->isActive = bridge->windowHasFocus;
	input->mouseWheelDelta = bridge->wheelThisFrame;

	fplMouseState mouseState = fplZeroInit;
	fplPollMouseState(&mouseState);
	input->mousePosition = fuiV2((float)mouseState.x, (float)mouseState.y);

	// FPL orders its buttons left, right, middle; final_ui.h orders them left, middle, right.
	const fplMouseButtonType fplButtonForFuiButton[FUI_MOUSE_BUTTON_COUNT] = {
		fplMouseButtonType_Left, fplMouseButtonType_Middle, fplMouseButtonType_Right,
	};
	for(int32_t buttonIndex = 0; buttonIndex < FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		bool isDown = mouseState.buttonStates[fplButtonForFuiButton[buttonIndex]] != fplButtonState_Release;
		bool wasDown = bridge->mouseWasDown[buttonIndex];
		input->mouseButtons[buttonIndex].endedDown = isDown;
		input->mouseButtons[buttonIndex].halfTransitionCount = (isDown != wasDown) ? 1 : 0;
		bridge->mouseWasDown[buttonIndex] = isDown;
	}

	fplKeyboardState keyboardState = fplZeroInit;
	fplPollKeyboardState(&keyboardState);
	for(int32_t keyIndex = 0; keyIndex < (int32_t)FUI_KEY_COUNT; ++keyIndex) {
		input->keys[keyIndex].endedDown = false;
		input->keys[keyIndex].halfTransitionCount = 0;
	}
	for(uint32_t rawKey = 0; rawKey < fplArrayCount(keyboardState.buttonStatesMapped); ++rawKey) {
		fuiKey mappedKey = MapFplKeyToFuiKey((fplKey)rawKey);
		if(mappedKey == FUI_KEY_NONE) {
			continue;
		}
		bool isDown = keyboardState.buttonStatesMapped[rawKey] != fplButtonState_Release;
		// Several raw keys can land on one fui key, so a key already reported down stays down.
		if(isDown) {
			input->keys[mappedKey].endedDown = true;
		}
	}
	for(int32_t keyIndex = 0; keyIndex < (int32_t)FUI_KEY_COUNT; ++keyIndex) {
		bool isDown = input->keys[keyIndex].endedDown;
		bool wasDown = bridge->keyWasDown[keyIndex];
		input->keys[keyIndex].halfTransitionCount = (isDown != wasDown) ? 1 : 0;
		bridge->keyWasDown[keyIndex] = isDown;
	}

	for(int32_t typedIndex = 0; typedIndex < bridge->typedCount; ++typedIndex) {
		input->textInput[typedIndex] = bridge->typedCodePoints[typedIndex];
	}
	input->textInputLength = bridge->typedCount;
}

// ----------------------------------------------------------------------------
// FPL's clipboard, as the library asks for it
// ----------------------------------------------------------------------------

static bool DemoGetClipboardText(void *userData, char *destination, uint32_t maxDestinationLength) {
	(void)userData;
	return fplGetClipboardText(destination, maxDestinationLength);
}

static bool DemoSetClipboardText(void *userData, const char *text) {
	(void)userData;
	return fplSetClipboardText(text);
}

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
			DemoSay(demo, "final_ui.h: one header, no dependencies. This window is FPL, OpenGL 1.1 and about 500 lines.");
		}
	}
	fuiEndMenu(ui);

	fuiEndMenuBar(ui);
}

static void BuildToolStrip(fuiContext *ui, DemoState *demo, const fuiRect stripRect) {
	fuiBeginToolStrip(ui, "toolstrip", stripRect, FUI_AXIS_HORIZONTAL);
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
	if(fuiBeginScrollPanelClosable(ui, "Widgets", FUI_DOCK_NONE, 24.0f, 110.0f, 360.0f, 470.0f, &demo->showWidgetsPanel)) {
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
	if(fuiBeginPanelClosable(ui, "Colour", FUI_DOCK_NONE, 410.0f, 110.0f, 420.0f, 300.0f, &demo->showPickerPanel)) {
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
	fuiBeginStackAt(ui, "previewcontrols", FUI_AXIS_HORIZONTAL, controlsRect, FUI_SPACING_FROM_THEME);
	for(int32_t modeIndex = 0; modeIndex < (int32_t)fplArrayCount(scaleModeNames); ++modeIndex) {
		(void)fuiRadio(ui, fuiLayoutSlot(ui, 110.0f), scaleModeNames[modeIndex], &demo->previewScaleMode, modeIndex);
	}
	fuiEndStack(ui);

	fuiRect flagsRect = fuiRectMake(contentRect.x, contentRect.y + DEMO_ROW_HEIGHT + 4.0f, contentRect.w, DEMO_ROW_HEIGHT);
	fuiBeginStackAt(ui, "previewflags", FUI_AXIS_HORIZONTAL, flagsRect, FUI_SPACING_FROM_THEME);
	(void)fuiCheckbox(ui, fuiLayoutSlot(ui, 140.0f), "Mirror", &demo->previewIsMirrored);
	(void)fuiCheckbox(ui, fuiLayoutSlot(ui, 190.0f), "Quarter turn", &demo->previewIsTurned);
	fuiEndStack(ui);

	// The whole sheet, drawn into what is left of the panel. The image is purely visual: it hit tests
	// nothing, so the checkboxes above it keep working right up to its edge.
	float boxTop = flagsRect.y + DEMO_ROW_HEIGHT + 8.0f;
	fuiRect box = fuiRectMake(contentRect.x, boxTop, contentRect.w, contentRect.y + contentRect.h - boxTop);
	fuiDrawRectOutline(ui, box, fuiGetTheme(ui)->panelBorderColor, 1.0f);

	fuiImageFlags flags = FUI_IMAGE_FLAGS_NONE;
	if(demo->previewIsMirrored) {
		flags = (fuiImageFlags)(flags | FUI_IMAGE_FLIP_U);
	}
	if(demo->previewIsTurned) {
		flags = (fuiImageFlags)(flags | FUI_IMAGE_ROTATE_90_CW);
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
	if(fuiBeginPanelClosable(ui, "Entity table", FUI_DOCK_NONE, 300.0f, 300.0f, DEMO_TABLE_PANEL_WIDTH, DEMO_TABLE_PANEL_HEIGHT, &demo->showTablePanel)) {
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
	if(fuiBeginScrollPanelClosable(ui, "Scrolling list", FUI_DOCK_NONE, 860.0f, 110.0f, 290.0f, 340.0f, &demo->showListPanel)) {
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
	fuiDialogResult discardResult = fuiMessageBox(ui, "discard", "Discard changes", "The level has unsaved changes. Discard them?", FUI_MESSAGE_BOX_YES_NO_CANCEL);
	if(discardResult == FUI_DIALOG_RESULT_YES) {
		DemoSay(demo, "Discarded. Enter took the first button, escape the dismissing one.");
	} else if(discardResult == FUI_DIALOG_RESULT_NO) {
		DemoSay(demo, "Kept. A message box closes itself as soon as it is answered.");
	} else if(discardResult == FUI_DIALOG_RESULT_CANCEL) {
		DemoSay(demo, "Cancelled.");
	}

	fuiDialogResult renameResult = fuiInputBox(ui, "rename", "Rename level", "New name", demo->renameField, (int32_t)fplArrayCount(demo->renameField));
	if(renameResult == FUI_DIALOG_RESULT_OK) {
		fplCopyString(demo->renameField, demo->nameField, fplArrayCount(demo->nameField));
		DemoSay(demo, "Renamed. The field takes the keyboard as the dialog opens, so you can just type.");
	}

	// The live update switch is the CALLER's: the dialog owns its row and nothing else, which is the only
	// way a colour that drives something real can be judged while it is being dragged.
	fuiDialogResult tintResult = fuiColorDialogEx(ui, "tint", "Pick a colour", &demo->dialogColor, true, &demo->colorLiveUpdate);
	if(tintResult == FUI_DIALOG_RESULT_OK) {
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
	if(browserResult == FUI_FILE_BROWSER_DESCEND) {
		// The dialog stays open and the caller lists the folder instead. Which folder is answered from the
		// ROW that was activated, not from where we happen to be: row zero of the levels listing is the ".."
		// that goes back up. Resetting the selection matters too, or the new listing opens with whatever row
		// index the old one was sitting on already highlighted.
		bool wentUpADirectory = isInsideLevels && (browserOutIndex == 0);
		demo->browserFolder = wentUpADirectory ? DEMO_BROWSER_ROOT : DEMO_BROWSER_LEVELS;
		demo->browserSelection = -1;
	} else if(browserResult == FUI_FILE_BROWSER_ACCEPT) {
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

	fuiDialogResult overwriteResult = fuiMessageBox(ui, "overwrite", "Overwrite", "That file already exists. Overwrite it?", FUI_MESSAGE_BOX_YES_NO);
	if(overwriteResult == FUI_DIALOG_RESULT_YES) {
		fuiCloseDialog(ui, "browse");
		DemoSay(demo, "Overwritten. That prompt was a modal on top of a modal.");
	} else if(overwriteResult == FUI_DIALOG_RESULT_NO) {
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
	fuiEndStatusBar(ui);
}

static void BuildUserInterface(fuiContext *ui, DemoState *demo, const bool rightWasPressed) {
	// The three bars are cut off the root container BEFORE anything else, so their width comes from whatever
	// encloses them and their thickness from the theme. Nothing here names a pixel: a menu bar is one menu
	// row tall, a tool strip one strip button, a status bar one line of text - and a restyled theme moves all
	// three without a constant to chase.
	fuiRect menuBarRect = fuiLayoutDock(ui, FUI_DOCK_TOP, fuiMenuBarHeight(ui));
	fuiRect toolStripRect = fuiLayoutDock(ui, FUI_DOCK_TOP, fuiToolStripThickness(ui));
	fuiRect statusBarRect = fuiLayoutDock(ui, FUI_DOCK_BOTTOM, fuiStatusBarHeight(ui));

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

	BuildStatusBar(ui, demo, statusBarRect);

	// Popups float ABOVE the docked layout but are BUILT last, which is what lets a later call take the
	// cursor from an earlier one with no z ordering anywhere.
	BuildMenuBar(ui, demo, menuBarRect);

	if(fuiBeginContextMenu(ui, "canvasmenu")) {
		if(fuiMenuItem(ui, "Bring the panels back", fpl_null, true)) {
			demo->showWidgetsPanel = true;
			demo->showPickerPanel = true;
			demo->showListPanel = true;
			demo->showTablePanel = true;
			DemoSay(demo, "All four panels are open again.");
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
	platform.getClipboardText = DemoGetClipboardText;
	platform.setClipboardText = DemoSetClipboardText;
	fuiSetPlatform(&ui, &platform);

	DemoState demo;
	DemoInit(&demo);

	// The second texture, and the only asset in the demo that is not a font: four icon cells the demo draws
	// itself. A failed upload leaves the sheet at zero, which is a list of plain text rows rather than a
	// reason not to start.
	unsigned char iconPixels[DEMO_ICON_SHEET_WIDTH * DEMO_ICON_SHEET_HEIGHT];
	DemoDrawIconSheet(iconPixels);
	uint32_t iconTexture = 0;
	if(fuiGL1UploadFontAtlas(iconPixels, DEMO_ICON_SHEET_WIDTH, DEMO_ICON_SHEET_HEIGHT, &iconTexture)) {
		demo.iconSheet = (fuiTextureId)iconTexture;
		demo.iconSheetSize = fuiV2((float)DEMO_ICON_SHEET_WIDTH, (float)DEMO_ICON_SHEET_HEIGHT);
	}

	InputBridge bridge;
	InputBridgeInit(&bridge);

	float smoothedFrameTime = 1.0f / 60.0f;
	while(demo.isRunning && fplWindowUpdate()) {
		InputBridgePumpEvents(&bridge);
		InputBridgeBuild(&bridge);

		// Smoothed, because a per-frame reciprocal flickers too fast to read.
		float frameTime = bridge.input.deltaTime;
		if(frameTime > 0.0f) {
			smoothedFrameTime = smoothedFrameTime * 0.9f + frameTime * 0.1f;
		}
		demo.framesPerSecond = (smoothedFrameTime > 0.0f) ? (1.0f / smoothedFrameTime) : 0.0f;

		// One pass: build the whole interface between begin and end, and let the library work out what
		// was clicked. That is the entire contract.
		fuiBeginFrame(&ui, &bridge.input, FUI_PASS_BOTH);
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
	fuiGL1DeleteTexture(atlasTexture);
	fuiStbttFontRelease(&bakedFont);
	fglUnloadOpenGL();
	fplPlatformRelease();
	return 0;
}
