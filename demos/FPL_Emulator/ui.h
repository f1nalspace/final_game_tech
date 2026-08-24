/*
Name:
	Final Gamebox

	Frontend-Part (UI Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.

	It is the glue between final_ui.h and this application: the font it draws with, the theme it
	draws in, the translation of platform input into a fuiInput, and the handful of composite
	widgets the library has no equivalent for.

	The geometry final_ui.h produces is drawn by RendererDrawUIDrawData in render.h, because that
	is where the OpenGL loader and every other draw call lives.
*/

#pragma once

#include <final_platform_layer.h>

#include <final_memory.h>

#include <final_math.h>

#include <final_ui.h>

#include <fui_font_stbtt.h>

#include "utils.h"

#include "render.h"

//
// Font
//
// The atlas carries no oversampling, so how crisp text looks is decided by how far the size it is drawn
// at sits from the size it was baked at. One atlas covers the body text and one covers the oversized panel
// watermarks, rather than stretching a single bake across a four times size range and blurring both ends.
//

// One baked size plus the atlas texture it was uploaded into
typedef struct {
	fuiStbttFont baked;
	Texture atlasTexture;
	// The context keeps a POINTER to this, so it lives here and not on a caller's stack
	fuiFont font;
} UIFontAtlas;

typedef struct {
	UIFontAtlas body;
	UIFontAtlas large;
} UIFont;

extern bool UIFontCreate(UIFont *font, const void *trueTypeData);
extern void UIFontRelease(UIFont *font);

//
// Theme
//

extern void UIApplyDarkTheme(fuiContext *ui, const float fontHeight, const float lineHeight);

//
// Platform services (clipboard and mouse cursor)
//

extern void UIInstallPlatform(fuiContext *ui);

//
// Input
//

// Clears everything that is only true for one frame, while leaving held buttons and keys down
extern void UIInputBeginFrame(fuiInput *input);

extern void UIInputSetButton(fuiButtonState *button, const bool isDown);

extern fuiKey UIKeyFromPlatformKey(const fplKey key);

extern void UIInputAddText(fuiInput *input, const uint32_t codePoint);

//
// Immediate drawing
//
// Thin adapters over the final_ui.h drawing calls, so the state panels can keep expressing
// themselves as "put this text at this spot" instead of being rebuilt around a layout container.
// Positions are top-left corners, in pixels, like everything else the library takes.
//

extern fuiColor UIColorFrom4f(const Color4f color);

extern void UIPanel(fuiContext *ui, const fuiRect rect, const bool isSunken);

extern void UIText(fuiContext *ui, const float x, const float y, const fuiColor color, const char *text, const size_t textLen);

extern fuiVec2 UITextSize(fuiContext *ui, const char *text, const size_t textLen);

// The oversized grey caption a state panel carries in its middle, such as "CPU" or "PPU". It swaps to the
// large atlas for the duration, the way the hand rolled UI swapped to its own large font
extern void UIWatermark(fuiContext *ui, const UIFont *font, const fuiRect rect, const char *text);

//
// Source list
//
// A scrolling list of text rows with a highlighted row that is independent of the selected one, and
// the ability to be scrolled to a row on request. That request is what lets the disassembly follow
// the program counter, which is the reason this is not a fuiListBox.
//

typedef struct {
	float scroll;
	int32_t selectedIndex;
} UISourceListState;

// highlightIndex marks a row without selecting it, minus one for none. scrollToIndex is a ONE SHOT request
// to bring a row into view, minus one when there is none - a standing request would fight the user's own drag.
extern void UISourceList(fuiContext *ui, const fuiRect rect, const char *id, const StringList *lines, UISourceListState *state, const int32_t highlightIndex, const int32_t scrollToIndex);

//
// Checkbox and radio with an enabled state
//
// fuiCheckbox and fuiRadio are always live and always take the rectangle they are given. These add
// the disabled state and the measured width that a row of them packed left to right needs.
//

extern float UICheckboxWidth(fuiContext *ui, const char *label);

// Where a checkbox or radio rect has to start for its BOX to sit on the same column as plain text drawn at
// contentX. The widget insets its own box by the theme's widget padding, which would otherwise make every
// checkbox row look indented against the text lines above it.
extern float UICheckboxRowX(fuiContext *ui, const float contentX);

extern bool UICheckboxEx(fuiContext *ui, const fuiRect rect, const char *label, bool *value, const bool enabled);

extern bool UIRadioEx(fuiContext *ui, const fuiRect rect, const char *label, int32_t *selected, const int32_t option, const bool enabled);
