/*
Name:
	Final Gamebox

	Frontend-Part (UI Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#pragma once

#include <final_platform_layer.h>

#include <final_fontloader.h>

#include <final_memory.h>

#include <final_math.h>

#include "utils.h"

#include "render.h"

//
// Style
//

typedef struct {
	Color4f normal;
	Color4f hover;
	Color4f down;
	Color4f disabled;
} UIColorStates;

typedef struct {
	Color4f bright;
	Color4f dark;
	Color4f darkest;
	uint8_t padding[16];
} UIBorderStyle;

typedef struct {
	UIColorStates background;
	UIColorStates foreground;
} UIControlStyle;

typedef struct {
	Color4f background;
	Color4f foreground;
} UILineStyle;

typedef struct {
	UIColorStates background;
	UIColorStates foreground;
	UILineStyle highlight;
} UIListboxStyle;

typedef struct {
	Color4f changing;
	Color4f checked;
	Color4f notChecked;
} UICheckboxStyle;

typedef struct {
	UIControlStyle face;
	UIListboxStyle listbox;
	UICheckboxStyle checkbox;
	UIBorderStyle buttonBorder;
} UIThemeStyle;

typedef enum {
	UITheme_Light = 0,
	UITheme_Dark,
} UITheme;

//
// Input
//

typedef struct {
	int32_t halfTransitionCount;
	uint32_t endedDown;
} UIButtonState;

inline bool UIWasPressed(const UIButtonState *state) {
	bool result = ((state->halfTransitionCount > 1) || ((state->halfTransitionCount == 1) && (!state->endedDown)));
	return(result);
}

inline bool UIIsDown(const UIButtonState *state) {
	bool result = state->endedDown != 0;
	return(result);
}

typedef struct {
	Mat4f projectionMat;
	Mat4f viewMat;
	Viewport4i viewport;
	Vec2f mousePos;
	UIButtonState leftMouse;
	UIButtonState middleMouse;
	UIButtonState rightMouse;
	UIButtonState escapeButton;
	float mouseWheelDelta;
	uint32_t padding;
} UIInputState;

//
// Base
//

typedef uintptr_t UIIdentifier;

extern UIIdentifier UIPtrToID(const void *ptr);

typedef struct {
	UIIdentifier id;
	UIIdentifier parentId;
	const char *name;
} UIBase;

//
// Common
//

typedef struct {
	float x;
	float y;
	float w;
	float h;
} UIRectangle;

typedef struct {
	const LoadedFont *currentFont;
	TextureID currentFontTextureID;
	uint32_t padding;
	float fontHeight;
	float lineHeight;
} UIFont;

//
// Interaction
//

typedef struct {
	Vec2f startPos;
	bool isActive;
	uint8_t padding1[3];
	uint32_t padding2;
} UIDraggingState;

typedef struct {
	bool isInteractable;
	bool isHover;
	bool isPressed;
	uint8_t padding;
} UIMouseInteraction;

//
// Window
//

typedef struct {
	UIBase base;
	Vec2f pos;
	Vec2f size;
} UIWindowData;

typedef struct UIWindowNode {
	UIWindowData data;
	struct UIWindowNode *prev;
	struct UIWindowNode *next;
} UIWindowNode;

typedef struct {
	UIWindowNode buffer[32];
	UIWindowNode *firstFree;
	UIWindowNode *head;
	UIWindowNode *tail;
	size_t used;
} UIWindowNodeList;

//
// Context
//

typedef struct {
	UIInputState input;
	UIFont font;
	const UIThemeStyle *style;
	UITheme theme;

	UIWindowNodeList windowList;
	UIWindowData rootWindow;
	UIIdentifier activeWindowId;
	bool isActive;
} UIContext;

extern void UIBegin(UIContext *ctx);
extern void UIEnd(UIContext *ctx);

extern void UIInitContext(UIContext *ctx, const UITheme initialTheme);

extern void UIContextSetInput(UIContext *ctx, const UIInputState *newState);

extern void UISetTheme(UIContext *ctx, const UITheme theme);
extern void UISetFont(UIContext *ctx, const LoadedFont *font, const TextureID texture, const float fontHeight, const float lineHeight);
extern void UIResetFont(UIContext *ctx, const UIFont *font);
extern UIFont UIGetFont(const UIContext *ctx);

extern float UIGetFontHeight(const UIContext *ctx);
extern float UIGetLineHeight(const UIContext *ctx);

extern Color4f UIGetForegroundColor(const UIContext *ctx);

//
// Panel
//

extern void UIPanel(const UIContext *ctx, const float x, const float y, const float w, const float h, const bool isDown);

//
// Panel
//

extern void UIPanel(const UIContext *ctx, const float x, const float y, const float w, const float h, const bool isDown);

//
// String
//

extern void UIString(const UIContext *ctx, const float x, const float y, const Color4f color, const char *text, const size_t textLen);
extern Vec2f UIGetStringSize(const UIContext *ctx, const char *text, const size_t textLen);

//
// Listbox
//

typedef struct {
	size_t index;
	bool hasRequest;
	uint8_t padding[7];
} UIListboxScrollRequest;

typedef struct {
	UIBase base;
	StringList values;
	UIListboxScrollRequest scrollRequest;
	UIDraggingState dragging;
	size_t highlightLineNum;
	uint64_t padding;
	float verticalScrollOffset;
	float lastVerticalScrollOffset;
} UIListboxData;

extern void UIListboxScrollTo(UIListboxData *listbox, const size_t index);
extern void UIListboxHighlight(UIListboxData *listbox, const size_t lineNum);
extern void UIListbox(UIContext *ctx, UIListboxData *listbox, const float x, const float y, const float w, const float h, const char *name);

//
// Button
//

typedef struct {
	UIBase base;
	UIMouseInteraction mouseInteraction;
} UIButtonData;

extern bool UIButton(UIContext *ctx, UIButtonData *button, const float x, const float y, const float w, const float h, const char *label, const size_t labelLen, const bool isEnabled);

typedef struct {
	UIBase base;
	const char *label;
} UITab;

extern UITab UICreateTab(const UIIdentifier id, const char *label);

typedef struct {
	UIBase base;
	const UITab *activeTab;
	const UITab *tabToSwitch;
	uint32_t tabCount;
	bool isInteractable;
} UITabControlData;

typedef struct {
	const UITab *activeTab;
	UIRectangle area;
} UITabContent;

extern UITabContent UITabControl(UIContext *ctx, UITabControlData *tabControl, const float x, const float y, const float w, const float h, const char *name, const UITab **tabs, const uint8_t tabCount);

//
// Checkbox
//

typedef struct {
	UIBase base;
	UIMouseInteraction mouseInteraction;
	float currentWidth;
} UICheckboxData;

extern bool UICheckbox(UIContext *ctx, UICheckboxData *checkbox, const float x, const float y, const char *label, const bool showLabel, const bool isChecked, const bool isEnabled);

//
// Dialog
//

typedef enum {
	UIDialogModalResult_None = 0,
	UIDialogModalResult_Cancel,
	UIDialogModalResult_OK,
} UIDialogModalResult;

typedef struct {
	UIBase base;
	UIWindowData window;
	UIDialogModalResult result;
	bool isShown;
	bool isActive;
} UIDialogData;

extern bool UIBeginDialog(UIContext *ctx, UIDialogData *dialog, const float w, const float h, const char *name);
extern void UIEndDialog(UIContext *ctx, UIDialogData *dialog);