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

//
// Relief and bevel box
//
// final_ui.h draws every widget box as a shaded face between a lit and a shaded edge, but keeps the painter
// that does it private. The composite widgets below carry their own copy of it, so a tab or a button built
// here stands in the same light as the ones the library builds.
//

typedef enum {
	// Standing out of the panel, which is what a button at rest and the tab in front look like
	UIRelief_Raised = 0,
	// Pushed into the panel, which is what a widget being held looks like
	UIRelief_Sunken,
	// Neither, just a filled box, which is what sits behind the front surface rather than on it
	UIRelief_Flat,
} UIRelief;

// Opacity multiplies every colour the box is drawn with, which is what lets a button fade out without the
// theme knowing anything about it. Pass one to draw it as the theme says.
extern void UIBevelBox(fuiContext *ui, const fuiRect rect, const fuiColor faceColor, const UIRelief relief, const float opacity);

//
// Tab strip
//
// The same row of headers fuiTabControl draws, except which tab is showing lives in the CALLER's state
// rather than inside the library. That is the whole reason this exists: a button that opens a dialog on a
// particular page has to be able to say which page, and the library's own tab control cannot be told.
//

extern int32_t UITabStrip(fuiContext *ui, const fuiRect rect, const char *id, const char *const *tabs, const int32_t tabCount, int32_t *selectedIndex);

//
// Text view
//
// A read only box of text that is NOT wrapped: every line is drawn exactly as it was written and the box
// scrolls in both directions to reach the rest of it. The text it is given is laid out in columns, so
// wrapping it would be the very thing that made it unreadable.
//

typedef struct {
	// What the measurement below was taken from, so a block of text is only measured again when it changes
	const char *measuredText;
	float measuredFontHeight;
	float contentWidth;
	float contentHeight;

	float scrollX;
	float scrollY;
} UITextViewState;

// backgroundOpacity fades the box's own fill, so something drawn BEHIND the view can show through the text
// sitting on it. Pass one for the solid box the theme describes, and zero to leave the fill out altogether.
extern void UITextView(fuiContext *ui, const fuiRect rect, const char *id, const char *text, UITextViewState *state, const float backgroundOpacity);

//
// Icon button
//
// A picture that is clickable and nothing else: no box, no bevel and no caption, so what the user sees is
// the icon itself. It answers a press by sinking slightly, which is the only chrome it carries.
//

// Opacity fades the icon, so one sitting over the emulated display can stay out of the way until the cursor
// comes near it. A faded icon is still fully clickable, and hovering it brings it fully back.
// The texture is loaded upside down by the image loader, which is why this flips it back on the way in.
extern bool UIIconButton(fuiContext *ui, const fuiRect rect, const char *id, const Texture *icon, const float opacity);
