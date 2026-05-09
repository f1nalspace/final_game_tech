/*
Name:
	Final Gamebox

	Frontend-Part (UI Implementation)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#include "ui.h"

//
// Style
//

#define DEBUG_RENDER_FONT_LINES 0

#define INV_COLOR_BYTE (1.0f / 255.0f)

#define RGB4F(r, g, b) { INV_COLOR_BYTE * (float)(r), INV_COLOR_BYTE * (float)(g), INV_COLOR_BYTE * (float)(b), 1.0f }

static const UIThemeStyle LightThemeStyle = {
	.face = {
		.background = {
			.normal = { 0.83f, 0.82f, 0.78f, 1.0f },
			.hover = { 0.93f, 0.93f, 0.93f, 1.0f },
			.down = { 0.83f, 0.82f, 0.78f, 1.0f },
			.disabled = { 0.33f, 0.32f, 0.38f, 1.0f },
		},
		.foreground = {
			.normal = { 0.0f, 0.0f, 0.0f, 1.0f },
			.hover = { 0.0f, 0.0f, 0.0f, 1.0f },
			.down = { 0.0f, 0.0f, 0.0f, 1.0f },
			.disabled = { 0.25f, 0.25f, 0.25f, 1.0f },
		},
	},
	.listbox = {
		.background = {
			.normal = { 1.0f, 1.0f, 1.0f, 1.0f },
			.hover = { 1.0f, 1.0f, 1.0f, 1.0f },
			.down = { 1.0f, 1.0f, 1.0f, 1.0f },
			.disabled = { 0.5f, 0.5f, 0.5f, 1.0f },
		},
		.foreground = {
			.normal = { 0.0f, 0.0f, 0.0f, 1.0f },
			.hover = { 0.0f, 0.0f, 0.0f, 1.0f },
			.down = { 0.0f, 0.0f, 0.0f, 1.0f },
			.disabled = { 0.25f, 0.25f, 0.25f, 1.0f },
		},
		.highlight = {
			.background = { 0.25f, 0.25f, 0.75f, 1.0f },
			.foreground = { 1.0f, 1.0f, 1.0f, 1.0f },
		},
	},
	.checkbox = {
		.changing = { 0.75f, 0.75f, 0.75f, 1.0f },
		.checked = { 0.0f, 0.0f, 0.0f, 1.0f },
		.notChecked = { 0.73f, 0.72f, 0.68f, 1.0f },
	},
	.buttonBorder = {
		.bright = { 1.0f, 1.0f, 1.0f, 1.0f },
		.dark = { 0.5f, 0.5f, 0.5f, 1.0f },
		.darkest = { 0.25f, 0.25f, 0.25f, 1.0f },
	}
};

static const UIThemeStyle DarkThemeStyle = {
	.face = {
		.background = {
			.normal = RGB4F(31, 31, 31),
			.hover = RGB4F(51, 51, 51),
			.down = RGB4F(31, 31, 31),
			.disabled = RGB4F(31, 31, 31),
		},
		.foreground = {
			.normal = RGB4F(220, 220, 204),
			.hover = RGB4F(220, 220, 204),
			.down = RGB4F(220, 220, 204),
			.disabled = RGB4F(120, 120, 104),
		},
	},
	.listbox = {
		.background = {
			.normal = RGB4F(37, 37, 38),
			.hover = RGB4F(37, 37, 38),
			.down = RGB4F(37, 37, 38),
			.disabled = RGB4F(37, 37, 38),
		},
		.foreground = {
			.normal = RGB4F(220, 220, 204),
			.hover = RGB4F(220, 220, 204),
			.down = RGB4F(220, 220, 204),
			.disabled = RGB4F(120, 120, 104),
		},
		.highlight = {
			.background = { 0.25f, 0.25f, 0.75f, 1.0f },
			.foreground = { 1.0f, 1.0f, 1.0f, 1.0f },
		},
	},
	.checkbox = {
		.changing = { 0.25f, 0.25f, 0.25f, 1.0f },
		.checked = { 1.0f, 1.0f, 1.0f, 1.0f },
		.notChecked = RGB4F(51, 51, 51),
	},
	.buttonBorder = {
		.bright = { 0.75f, 0.75f, 0.75f, 0.75f },
		.dark = { 0.4f, 0.4f, 0.4f, 1.0f },
		.darkest = { 0.05f, 0.05f, 0.05f, 1.0f },
	}
};

//
// Helper
//

static UIRectangle MakeRectangle(const float x, const float y, const float w, const float h) {
	UIRectangle result = fplZeroInit;
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return result;
}

static bool IsPointInsideRectangle(const UIRectangle *rect, const float x, const float y) {
	bool result =
		(x >= rect->x && x <= (rect->x + rect->w)) &&
		(y >= rect->y && y <= (rect->y + rect->h));
	return result;
}

extern UIIdentifier UIPtrToID(const void *ptr) {
	return (UIIdentifier)ptr;
}

//
// Interaction
//

static bool UpdateDragging(const Vec2f mousePos, UIDraggingState *dragging, Vec2f *offset) {
	if (dragging->isActive) {
		*offset = V2fSub(mousePos, dragging->startPos);
		dragging->startPos = mousePos;
		return true;
	}
	return false;
}

static bool BeginDragging(const Vec2f mousePos, UIDraggingState *dragging) {
	if (!dragging->isActive) {
		dragging->startPos = mousePos;
		dragging->isActive = true;
		return true;
	}
	return false;
}

static bool StopDragging(UIDraggingState *dragging) {
	if (dragging->isActive) {
		dragging->isActive = false;
		return true;
	}
	return false;
}

static bool UI__MouseInteraction(UIContext *ctx, UIMouseInteraction *interaction, const UIRectangle *rect, const bool isEnabled) {
	bool wasPressed = false;

	bool isPressing = UIIsDown(&ctx->input.leftMouse);

	bool isMouseInside = IsPointInsideRectangle(rect, ctx->input.mousePos.x, ctx->input.mousePos.y);

	if (isEnabled) {
		if (isMouseInside) {
			interaction->isHover = true;

			if (!interaction->isInteractable) {
				interaction->isInteractable = true;
				interaction->isPressed = false;
			} else {
				if (!interaction->isPressed) {
					if (isPressing) {
						interaction->isPressed = true;
					}
				} else {
					if (!isPressing) {
						interaction->isPressed = false;
						wasPressed = true;
					}
				}
			}
		} else {
			if (interaction->isPressed) {
				if (!isPressing) {
					interaction->isPressed = false;
					interaction->isInteractable = false;
					interaction->isHover = false;
				}
			} else if (interaction->isInteractable) {
				interaction->isInteractable = false;
				interaction->isPressed = false;
			}
		}
	}

	return wasPressed;
}

//
// Window
//

static void UI__ResetWindowNodeList(UIWindowNodeList *list) {
	size_t capacity = fplArrayCount(list->buffer);
	fplAssert(capacity > 1);
	fplClearStruct(list);
	for (size_t i = 0; i < capacity - 1; ++i) {
		UIWindowNode *node = &list->buffer[i];
		UIWindowNode *nextNode = &list->buffer[i + 1];
		node->next = (struct UIWindowNode *)nextNode;
	}
	list->firstFree = &list->buffer[0];
	list->head = list->tail = fpl_null;
	list->used = 0;
}

static void UI__InitializeWindowNodeList(UIWindowNodeList *list) {
	UI__ResetWindowNodeList(list);
}

static UIWindowNode *UI__AddWindowNodeToFront(UIWindowNodeList *list) {
	UIWindowNode *node = list->firstFree;
	if (node == fpl_null) {
		return fpl_null;
	}
	fplAssert(list->used < fplArrayCount(list->buffer));
	list->firstFree = node->next;

	fplClearStruct(node);
	node->next = list->head;

	if (list->head != fpl_null) {
		list->head->prev = node;
	} else {
		list->tail = node;
	}

	list->head = node;

	++list->used;

	return node;
}

static UIWindowNode *UI__AddWindowNodeToBack(UIWindowNodeList *list) {
	UIWindowNode *node = list->firstFree;
	if (node == fpl_null) {
		return fpl_null;
	}
	fplAssert(list->used < fplArrayCount(list->buffer));
	list->firstFree = node->next;

	fplClearStruct(node);

	if (list->head == fpl_null) {
		list->head = list->tail = node;
	} else {
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}

	++list->used;

	return node;
}

static void UI__RemoveWindowNode(UIWindowNodeList *list, UIWindowNode *node) {
	fplAssert(list != fpl_null && node != fpl_null);
	fplAssert(list->used > 0);
	if (list->head == node) {
		list->head = node->next;
	}
	if (list->tail == node) {
		list->tail = node->prev;
	}
	if (node->prev != fpl_null) {
		node->prev->next = node->next;
	}
	if (node->next != fpl_null) {
		node->next->prev = node->prev;
	}

	fplClearStruct(node);
	node->next = list->firstFree;
	list->firstFree = node;

	--list->used;
}

static UIWindowNode *UI__FindWindowNode(UIContext *ctx, const UIIdentifier id) {
	UIWindowNode *node = ctx->windowList.head;
	while (node != fpl_null) {
		UIWindowNode *next = node->next;
		if (node->data.base.id == id) {
			return node;
		}
		node = next;
	}	
	return fpl_null;
}

static bool UI__ContainsWindow(UIContext *ctx, const UIIdentifier id) {
	UIWindowNode *node = UI__FindWindowNode(ctx, id);
	return node != fpl_null;
}

#define UI__WINDOW_ID_CONCAT(a, id, b) a # id # b

static UIWindowNode *UI__PushWindow(UIContext *ctx, const UIIdentifier id, const float w, const float h) {
	bool alreadyExists = UI__ContainsWindow(ctx, id);
	if (alreadyExists) {
		fplAssert(!UI__WINDOW_ID_CONCAT("Window by id ", id, " already exists!"));
		return fpl_null;
	}

	UIWindowNode *windowNode = UI__AddWindowNodeToFront(&ctx->windowList);
	fplAssert(windowNode != fpl_null);

	UIWindowNode *parentNode = windowNode->next;
	fplAssert(parentNode != fpl_null);
	
	UIWindowData parentWindow = parentNode->data;

	windowNode->data.base.id = id;
	windowNode->data.base.parentId = parentWindow.base.id;
	windowNode->data.pos.x = parentWindow.pos.x + (parentWindow.size.w - w) * 0.5f;
	windowNode->data.pos.y = parentWindow.pos.y + (parentWindow.size.h - h) * 0.5f;
	windowNode->data.size.w = w;
	windowNode->data.size.h = h;

	return windowNode;
}

//
// Common
//

extern void UIInitContext(UIContext *ctx, const UITheme initialTheme) {
	if (ctx == fpl_null) {
		return;
	}
	fplClearStruct(ctx);
	if (initialTheme == UITheme_Dark)
		ctx->style = &DarkThemeStyle;
	else
		ctx->style = &LightThemeStyle;
	ctx->theme = initialTheme;

	if (initialTheme == UITheme_Dark)
		ctx->style = &DarkThemeStyle;
	else
		ctx->style = &LightThemeStyle;

	ctx->theme = initialTheme;

	UI__InitializeWindowNodeList(&ctx->windowList);

	ctx->rootWindow.base.id = UIPtrToID(&ctx->rootWindow);
	ctx->rootWindow.base.name = "ROOT";

	ctx->activeWindowId = ctx->rootWindow.base.id;
}

extern void UISetTheme(UIContext *ctx, const UITheme theme) {
	if (ctx == fpl_null) {
		return;
	}
	if (theme == UITheme_Dark)
		ctx->style = &DarkThemeStyle;
	else
		ctx->style = &LightThemeStyle;
	ctx->theme = theme;
}

extern void UIContextSetInput(UIContext *ctx, const UIInputState *newState) {
	if (ctx == fpl_null || newState == fpl_null) {
		return;
	}
	ctx->input = *newState;
}

extern void UISetFont(UIContext *ctx, const LoadedFont *font, const TextureID texture, const float fontHeight, const float lineHeight) {
	if (ctx == fpl_null) {
		return;
	}

	UIFont ufont = fplZeroInit;
	ufont.currentFont = font;
	ufont.currentFontTextureID = texture;
	ufont.fontHeight = fontHeight;
	ufont.lineHeight = lineHeight;

	ctx->font = ufont;
}

extern void UIResetFont(UIContext *ctx, const UIFont *font) {
	if (ctx == fpl_null || font == fpl_null) {
		return;
	}
	ctx->font = *font;
}

static UIFont UIEmptyFont = fplZeroInit;

extern UIFont UIGetFont(const UIContext *ctx) {
	if (ctx == fpl_null) {
		return UIEmptyFont;
	}
	return ctx->font;
}

extern float UIGetFontHeight(const UIContext *ctx) {
	if (ctx == fpl_null)
		return 0;
	return ctx->font.fontHeight;
}

extern float UIGetLineHeight(const UIContext *ctx) {
	if (ctx == fpl_null)
		return 0;
	return ctx->font.lineHeight;
}

extern Color4f UIGetForegroundColor(const UIContext *ctx) {
	fplAssert(ctx->style != fpl_null);
	return ctx->style->face.foreground.normal;
}

static bool UI__InitializeControl(UIContext *ctx, UIBase *base, const void *ptr, const char *name) {
	if (base->id == 0) {
		base->id = UIPtrToID(ptr);
		base->name = name;
	}

	UIWindowNode *parent = ctx->windowList.head;
	fplAssert(parent != fpl_null);

	base->parentId = parent->data.base.id;

	bool result = base->parentId == ctx->activeWindowId;
	return result;
}

extern void UIBegin(UIContext *ctx) {
	fplAssert(!ctx->isActive);

	ctx->rootWindow.pos = V2fInit(0, 0);
	ctx->rootWindow.size = V2fInit((float)ctx->input.viewport.w, (float)ctx->input.viewport.h);

	UI__ResetWindowNodeList(&ctx->windowList);

	UIWindowNode *node = UI__AddWindowNodeToFront(&ctx->windowList);
	fplAssert(node != fpl_null);
	node->data = ctx->rootWindow;

	ctx->isActive = true;

}

extern void UIEnd(UIContext *ctx) {
	fplAssert(ctx->isActive);

	fplAssert(ctx->windowList.used == 1);
	fplAssert(ctx->windowList.head->data.base.id == ctx->rootWindow.base.id);
	UI__RemoveWindowNode(&ctx->windowList, ctx->windowList.head);

	ctx->isActive = false;
}

//
// Panel
//

extern void UIPanel(const UIContext *ctx, const float x, const float y, const float w, const float h, const bool isDown) {
	fplAssert(ctx->style != fpl_null);
	const UIBorderStyle *buttonBorderStyle = &ctx->style->buttonBorder;
	const UIControlStyle *controlStyle = &ctx->style->face;

	const float borderThickness = 1.5f;

	RendererDrawFilledQuad(x, y, w, h, controlStyle->background.normal);
	RendererDrawControlBorder(x, y, w, h, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, isDown);
}

//
// String
//

extern void UIString(const UIContext *ctx, const float x, const float y, const Color4f color, const char *text, const size_t textLen) {
	const TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const UIFont *font = &ctx->font;

	const LoadedFont *loadedFont = font->currentFont;
	fplAssert(loadedFont != fpl_null);

	const float fontHeight = UIGetFontHeight(ctx);

	const size_t actualLen = textLen > 0 ? textLen : fplGetStringLength(text);

	RendererDrawString(loadedFont, fontTextureId, text, actualLen, x, y, fontHeight, color);

#if DEBUG_RENDER_FONT_LINES
	const Color4f lineColorDescent = fplStructInit(Color4f, 1.0f, 0.0f, 0.0f, 1.0f);
	const Color4f lineColorAscent = fplStructInit(Color4f, 0.0f, 1.0f, 0.0f, 1.0f);
	const float descent = loadedFont->info.descent * fontHeight;
	const float ascent = loadedFont->info.ascent * fontHeight;
	const Vec2f s = FontGetTextSize(loadedFont, text, actualLen, fontHeight);
	RendererDrawLine(x, y - descent, x + s.w, y - descent, 1.0f, lineColorDescent);
	RendererDrawLine(x, y + ascent, x + s.w, y + ascent, 1.0f, lineColorAscent);
#endif
}

extern Vec2f UIGetStringSize(const UIContext *ctx, const char *text, const size_t textLen) {
	const TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const UIFont *font = &ctx->font;

	const LoadedFont *loadedFont = font->currentFont;
	fplAssert(loadedFont != fpl_null);

	const float fontHeight = UIGetFontHeight(ctx);

	const size_t actualLen = textLen > 0 ? textLen : fplGetStringLength(text);

	const Vec2f s = FontGetTextSize(loadedFont, text, actualLen, fontHeight);

	return fplStructInit(Vec2f, s.x, s.y);
}

//
// Listbox
//

extern void UIListboxScrollTo(UIListboxData *listbox, const size_t index) {
	listbox->scrollRequest.hasRequest = true;
	listbox->scrollRequest.index = index;
}

extern void UIListboxHighlight(UIListboxData *listbox, const size_t lineNum) {
	listbox->highlightLineNum = lineNum;
}

extern void UIListbox(UIContext *ctx, UIListboxData *listbox, const float x, const float y, const float w, const float h, const char *name) {
	bool allowInput = UI__InitializeControl(ctx, &listbox->base, listbox, name);

	fplAssert(ctx->style != fpl_null);
	const UIBorderStyle *buttonBorderStyle = &ctx->style->buttonBorder;
	const UIControlStyle *controlStyle = &ctx->style->face;
	const UIListboxStyle *listboxStyle = &ctx->style->listbox;

	TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const LoadedFont *font = ctx->font.currentFont;
	fplAssert(font != fpl_null);

	const float lineHeight = UIGetLineHeight(ctx);
	const float fontHeight = UIGetFontHeight(ctx);
	const float descent = font->info.descent * fontHeight;
	const float ascent = font->info.ascent * fontHeight;

	const float borderThickness = 1.5f;

	size_t maxVisibleLineCount = (size_t)(h / lineHeight);
	size_t visibleLineCount = fplMin(listbox->values.count, maxVisibleLineCount);

	UIRectangle listboxRect = MakeRectangle(x, y, w, h);

	RendererDrawFilledQuad(x, y, w, h, listboxStyle->background.normal);
	RendererDrawControlBorder(x, y, w, h, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, true);

	RendererEnableClipping();
	RendererSetViewClipRect(&ctx->input.projectionMat, &ctx->input.viewMat, &ctx->input.viewport, x, y, w, h);

	float availableWidth = w;

	float verticalScrollHeight = 0;
	float verticalScrollWidth = 40.0f;
	float smallestVerticalScrollbarHeight = h * 0.05f;
	float verticalScrollBarHeight = 0;

	size_t lineOffset = 0;

	// verticalScrollOffset stores first-visible-line index as float [0, missingLineCount].
	// Resize-stable: line index is independent of pixel height.
	float pixelScrollOffset = 0;

	bool scrollbarIsVisible = false;

	if (listbox->values.count > maxVisibleLineCount) {
		float overflowFactor = maxVisibleLineCount / (float)listbox->values.count;

		availableWidth = w - verticalScrollWidth;

		verticalScrollBarHeight = fplMax(h * overflowFactor, smallestVerticalScrollbarHeight);

		verticalScrollHeight = h - verticalScrollBarHeight;

		size_t missingLineCount = listbox->values.count - maxVisibleLineCount;
		size_t halfVisibleLineCount = maxVisibleLineCount / 2;

		listbox->lastVerticalScrollOffset = listbox->verticalScrollOffset;

		if (listbox->scrollRequest.hasRequest) {
			size_t targetIndex = listbox->scrollRequest.index;
			size_t currentFirst = (size_t)listbox->verticalScrollOffset;
			size_t currentLast = currentFirst + maxVisibleLineCount - 1;
			// Only scroll if target is outside the current visible range
			if (targetIndex < currentFirst || targetIndex > currentLast) {
				// Center the target; clamp to valid range
				size_t targetFirst = (targetIndex > halfVisibleLineCount)
					? (targetIndex - halfVisibleLineCount) : 0;
				if (targetFirst > missingLineCount)
					targetFirst = missingLineCount;
				listbox->verticalScrollOffset = (float)targetFirst;
			}
			listbox->scrollRequest.hasRequest = false;
			listbox->scrollRequest.index = 0;
		}

		// Clamp first-visible-line to [0, missingLineCount]
		listbox->verticalScrollOffset = fplMax(0.0f, fplMin(listbox->verticalScrollOffset, (float)missingLineCount));

		lineOffset = (size_t)listbox->verticalScrollOffset;

		// Derive pixel offset for thumb from line position
		pixelScrollOffset = (missingLineCount > 0)
			? ((float)lineOffset / (float)missingLineCount) * verticalScrollHeight
			: 0.0f;

		scrollbarIsVisible = true;
	} else {
		listbox->lastVerticalScrollOffset = listbox->verticalScrollOffset = 0.0f;
	}

	const Color4f lineColorDescent = fplStructInit(Color4f, 1.0f, 0.0f, 0.0f, 1.0f);
	const Color4f lineColorAscent = fplStructInit(Color4f, 0.0f, 1.0f, 0.0f, 1.0f);

	for (size_t visibleLineIndex = 0; visibleLineIndex < visibleLineCount; ++visibleLineIndex) {
		String *entry = listbox->values.entries + visibleLineIndex + lineOffset;
		size_t len = entry->len;
		const char *text = entry->text;

		Vec2f textSize = FontGetTextSize(font, text, len, fontHeight);

		bool isHighlight = (listbox->highlightLineNum > 0) && (visibleLineIndex + lineOffset) == (listbox->highlightLineNum - 1);

		float lineWidth = availableWidth;
		float lineX = x;
		float lineY = y + h - lineHeight - visibleLineIndex * lineHeight;
		float blockY = lineY - descent;
		float textY = lineY;

		if (isHighlight) {
			RendererDrawFilledQuad(lineX, blockY, lineWidth, lineHeight, listboxStyle->highlight.background);
			RendererDrawString(font, fontTextureId, text, len, x, textY, fontHeight, listboxStyle->highlight.foreground);
		} else {
			RendererDrawString(font, fontTextureId, text, len, x, textY, fontHeight, listboxStyle->foreground.normal);
		}

#if DEBUG_RENDER_FONT_LINES
		RendererDrawLine(x, textY + ascent, x + textSize.w, textY + ascent, 1.0f, lineColorAscent);
		RendererDrawLine(x, textY - descent, x + textSize.w, textY - descent, 1.0f, lineColorDescent);
#endif
	}

	Color4f scrollbarBackground = fplStructInit(Color4f, 0.25f, 0.25f, 0.25f, 1.0f);

	if (scrollbarIsVisible) {
		float vertScrollHeight = h;
		float vertScrollCenterX = x + w - verticalScrollWidth * 0.5f;
		float vertScrollCenterY = y + h * 0.5f;
		float vertScrollbarWidth = verticalScrollWidth * 1.0f;

		UIRectangle background = MakeRectangle(
			vertScrollCenterX - verticalScrollWidth * 0.5f,
			vertScrollCenterY - vertScrollHeight * 0.5f,
			verticalScrollWidth, vertScrollHeight);
		RendererDrawFilledQuad(background.x, background.y, background.w, background.h, scrollbarBackground);

		//float tempBarWidth = verticalScrollWidth * 0.5f;
		//DrawFilledQuad(vertScrollCenterX - tempBarWidth * 0.5f, vertScrollCenterY - verticalScrollHeight * 0.5f, tempBarWidth, verticalScrollHeight, 0.25f, 0.5f, 0.25f);

		UIRectangle scrollbar = MakeRectangle(
			vertScrollCenterX - vertScrollbarWidth * 0.5f,
			vertScrollCenterY - verticalScrollBarHeight * 0.5f + verticalScrollHeight * 0.5f - pixelScrollOffset,
			vertScrollbarWidth, verticalScrollBarHeight);

		bool isMouseInsideScrollbar = IsPointInsideRectangle(&scrollbar, ctx->input.mousePos.x, ctx->input.mousePos.y);

		if (allowInput) {
			if (listbox->dragging.isActive) {
				if (!UIIsDown(&ctx->input.leftMouse)) {
					StopDragging(&listbox->dragging);
				} else {
					Vec2f dragOffset;
					if (UpdateDragging(ctx->input.mousePos, &listbox->dragging, &dragOffset)) {
						size_t missingLineCount = listbox->values.count - maxVisibleLineCount;
						listbox->verticalScrollOffset -= dragOffset.y * (float)missingLineCount / verticalScrollHeight;
					}
				}
			}

			if (isMouseInsideScrollbar) {
				if (UIIsDown(&ctx->input.leftMouse)) {
					BeginDragging(ctx->input.mousePos, &listbox->dragging);
				}
			}

			bool isMouseInsideListbox = IsPointInsideRectangle(&listboxRect, ctx->input.mousePos.x, ctx->input.mousePos.y);

			if (isMouseInsideListbox && !listbox->dragging.isActive && ctx->input.mouseWheelDelta != 0.0f) {
				listbox->verticalScrollOffset -= 3.0f * ctx->input.mouseWheelDelta;
			}
		}

		// Scrollbar button
		const float scrollbarBorder = 2.0f;
		if (isMouseInsideScrollbar) {
			RendererDrawFilledQuad(scrollbar.x, scrollbar.y, scrollbar.w, scrollbar.h, controlStyle->background.hover);
		} else {
			RendererDrawFilledQuad(scrollbar.x, scrollbar.y, scrollbar.w, scrollbar.h, controlStyle->background.normal);
		}
		RendererDrawControlBorder(scrollbar.x, scrollbar.y, scrollbar.w, scrollbar.h, scrollbarBorder, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, listbox->dragging.isActive);
	} else {
		if (allowInput) {
			StopDragging(&listbox->dragging);
		}
	}

	RendererDisableClipping();
}

//
// Button
//

extern bool UIButton(UIContext *ctx, UIButtonData *button, const float x, const float y, const float w, const float h, const char *label, const size_t labelLen, const bool isEnabled) {
	bool allowInput = UI__InitializeControl(ctx, &button->base, button, label);

	const UIBorderStyle *buttonBorderStyle = &ctx->style->buttonBorder;
	const UIControlStyle *controlStyle = &ctx->style->face;

	const float fontHeight = UIGetFontHeight(ctx);

	const LoadedFont *font = ctx->font.currentFont;
	fplAssert(font != fpl_null);

	const float descent = font->info.descent * fontHeight;
	const float ascent = font->info.ascent * fontHeight;

	const float borderThickness = 1.5f;

	const TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const UIRectangle rect = MakeRectangle(x, y, w, h);

	bool wasPressed = allowInput && UI__MouseInteraction(ctx, &button->mouseInteraction, &rect, isEnabled);

	bool isDisabled = !isEnabled;
	bool isDown = button->mouseInteraction.isPressed;
	bool isHover = button->mouseInteraction.isHover;

	Color4f backgroundColor;
	Color4f foregroundColor;
	if (isDisabled) {
		backgroundColor = controlStyle->background.disabled;
		foregroundColor = controlStyle->foreground.disabled;
	} else {
		if (isHover) {
			if (isDown) {
				backgroundColor = controlStyle->background.down;
				foregroundColor = controlStyle->foreground.down;
			} else {
				backgroundColor = controlStyle->background.hover;
				foregroundColor = controlStyle->foreground.hover;
			}
		} else {
			backgroundColor = controlStyle->background.normal;
			foregroundColor = controlStyle->foreground.normal;
		}
	}

	RendererDrawFilledQuad(x, y, w, h, backgroundColor);
	RendererDrawControlBorder(x, y, w, h, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, !isDisabled && isDown);

	const Vec2f size = FontGetTextSize(font, label, labelLen, fontHeight);

	const float labelX = x + (w - size.w) * 0.5f;
	const float labelY = y + (h - size.h) * 0.5f;

	RendererDrawString(font, fontTextureId, label, labelLen, labelX, labelY, fontHeight, foregroundColor);

#if DEBUG_RENDER_FONT_LINES
	const Color4f lineColorDescent = fplStructInit(Color4f, 1.0f, 0.0f, 0.0f, 1.0f);
	const Color4f lineColorAscent = fplStructInit(Color4f, 0.0f, 1.0f, 0.0f, 1.0f);
	RendererDrawLine(labelX, labelY + ascent, labelX + size.w, labelY + ascent, 1.0f, lineColorAscent);
	RendererDrawLine(labelX, labelY - descent, labelX + size.w, labelY - descent, 1.0f, lineColorDescent);
#endif

	return wasPressed;
}

//
// TabControl
//

extern UITab UICreateTab(const UIIdentifier id, const char *label) {
	return (UITab) { .base.id = id, .label = label };
}

extern UITabContent UITabControl(UIContext *ctx, UITabControlData *tabControl, const float x, const float y, const float w, const float h, const char *name, const UITab **tabs, const uint8_t tabCount) {
	if (ctx == fpl_null || tabs == fpl_null || tabCount == 0) {
		return fplStructInit(UITabContent, 0);
	}

	bool allowInput = UI__InitializeControl(ctx, &tabControl->base, tabControl, name);

	const UIBorderStyle *buttonBorderStyle = &ctx->style->buttonBorder;
	const UIControlStyle *controlStyle = &ctx->style->face;

	if (tabControl->activeTab == fpl_null) {
		tabControl->activeTab = tabs[0];
	}

	const float contentPadding = 20.0f;

	const float margin = 0.0f;

	const float borderThickness = 1.5f;

	TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const float lineHeight = UIGetLineHeight(ctx);
	const float fontHeight = UIGetFontHeight(ctx);

	const LoadedFont *font = ctx->font.currentFont;
	fplAssert(font != fpl_null);

	const float descent = font->info.descent * fontHeight;
	const float ascent = font->info.ascent * fontHeight;

	const float controlWidth = w - margin * 2.0f;
	const float controlHeight = h - margin * 2.0f;
	const float controlX = x + margin;
	const float controlY = y + margin;

	UIRectangle controlRect = MakeRectangle(controlX, controlY, controlWidth, controlHeight);

	bool isMouseInside = IsPointInsideRectangle(&controlRect, ctx->input.mousePos.x, ctx->input.mousePos.y);
	if (allowInput) {
		if (isMouseInside) {
			if (!tabControl->isInteractable) {
				tabControl->isInteractable = true;
				tabControl->tabToSwitch = fpl_null;
			} else {
				if (tabControl->tabToSwitch != fpl_null) {
					tabControl->activeTab = tabControl->tabToSwitch;
					tabControl->tabToSwitch = fpl_null;
				}
			}
		} else {
			tabControl->isInteractable = false;
			tabControl->tabToSwitch = fpl_null;
		}
	}

	const float headerWidth = controlWidth;
	const float headerHeight = lineHeight * 1.5f;
	const float headerX = controlX;
	const float headerY = controlY + controlHeight - headerHeight;

	const float tabItemPadding = 10.0f;

	const float controlHeightWithoutHeader = controlHeight - headerHeight;

	RendererDrawFilledQuad(controlX, controlY, controlWidth, controlHeightWithoutHeader, controlStyle->background.normal);

	RendererDrawControlBorders(controlX, controlY, controlWidth, controlHeightWithoutHeader, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, ControlBorderFlags_Bottom | ControlBorderFlags_Left | ControlBorderFlags_Right, false);

	static UIRectangle tabAreas[8];
	fplClearStruct(tabAreas);

	// Compute tab area
	float tabHeaderItemX = headerX;
	float tabHeaderItemY = headerY;
	UIRectangle activeTabArea = fplZeroInit;
	for (uint8_t tabIndex = 0; tabIndex < tabCount; ++tabIndex) {
		const UITab *tab = tabs[tabIndex];

		UIRectangle *tabArea = tabAreas + tabIndex;

		size_t labelLen = fplGetStringLength(tab->label);

		Vec2f labelSize = FontGetTextSize(font, tab->label, labelLen, fontHeight);

		float tabHeaderItemWidth = labelSize.w + tabItemPadding * 2.0f;
		float tabHeaderItemHeight = headerHeight + 0.5f;

		fplClearStruct(tabArea);
		tabArea->x = tabHeaderItemX;
		tabArea->y = tabHeaderItemY;
		tabArea->w = tabHeaderItemWidth;
		tabArea->h = tabHeaderItemHeight;

		UIRectangle tabRect = MakeRectangle(tabHeaderItemX, tabHeaderItemY, tabHeaderItemWidth, tabHeaderItemHeight);

		bool allowHover = tab != tabControl->activeTab;

		bool isTabHover = IsPointInsideRectangle(&tabRect, ctx->input.mousePos.x, ctx->input.mousePos.y);

		if (allowInput && tabControl->isInteractable && isTabHover && allowHover) {
			if (UIIsDown(&ctx->input.leftMouse)) {
				tabControl->tabToSwitch = tab;
			}
		}

		if (isTabHover && allowHover) {
			RendererDrawFilledQuad(tabHeaderItemX, tabHeaderItemY, tabHeaderItemWidth, tabHeaderItemHeight, controlStyle->background.hover);
		} else {
			RendererDrawFilledQuad(tabHeaderItemX, tabHeaderItemY, tabHeaderItemWidth, tabHeaderItemHeight, controlStyle->background.normal);
		}

		RendererDrawControlBorders(tabHeaderItemX, tabHeaderItemY, tabHeaderItemWidth, tabHeaderItemHeight, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, ControlBorderFlags_Top | ControlBorderFlags_Left | ControlBorderFlags_Right, false);

		if (tabControl->activeTab == tab) {
			activeTabArea = *tabArea;
		} else {
			RendererDrawControlBorders(tabHeaderItemX, tabHeaderItemY, tabHeaderItemWidth, tabHeaderItemHeight, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, ControlBorderFlags_Bottom, true);
		}

		tabHeaderItemX += tabHeaderItemWidth;
	}

	const Color4f lineColorDescent = fplStructInit(Color4f, 1.0f, 0.0f, 0.0f, 1.0f);
	const Color4f lineColorAscent = fplStructInit(Color4f, 0.0f, 1.0f, 0.0f, 1.0f);

	for (uint8_t tabIndex = 0; tabIndex < tabCount; ++tabIndex) {
		const UITab *tab = tabs[tabIndex];
		UIRectangle *tabArea = tabAreas + tabIndex;

		size_t labelLen = fplGetStringLength(tab->label);

		Vec2f labelSize = FontGetTextSize(font, tab->label, labelLen, fontHeight);

		float tabHeaderLabelX = tabArea->x + tabItemPadding;
		float tabHeaderLabelY = tabArea->y + (tabArea->h - labelSize.h) * 0.5f;

		UIRectangle tabRect = MakeRectangle(tabArea->x, tabArea->y, tabArea->w, tabArea->h);

		bool allowHover = tab != tabControl->activeTab;

		bool isTabHover = IsPointInsideRectangle(&tabRect, ctx->input.mousePos.x, ctx->input.mousePos.y);

		Color4f foregroundColor;
		if (isTabHover && allowHover) {
			foregroundColor = controlStyle->foreground.hover;
		} else {
			foregroundColor = controlStyle->foreground.normal;
		}

		RendererDrawString(font, fontTextureId, tab->label, labelLen, tabHeaderLabelX, tabHeaderLabelY, fontHeight, foregroundColor);

#if DEBUG_RENDER_FONT_LINES
		RendererDrawLine(tabHeaderLabelX, tabHeaderLabelY + ascent, tabHeaderLabelX + labelSize.w, tabHeaderLabelY + ascent, 1.0f, lineColorAscent);
		RendererDrawLine(tabHeaderLabelX, tabHeaderLabelY - descent, tabHeaderLabelX + labelSize.w, tabHeaderLabelY - descent, 1.0f, lineColorDescent);
#endif
	}

	fplAssert(tabCount > 0);
	UIRectangle lastArea = tabAreas[tabCount - 1];

	const float hiddenBorderTabX = lastArea.x + lastArea.w;
	const float hiddenBorderTabWidth = (headerX + headerWidth) - (lastArea.x + lastArea.w);
	RendererDrawControlBorders(hiddenBorderTabX, tabHeaderItemY, hiddenBorderTabWidth, headerHeight, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, ControlBorderFlags_Bottom, true);

	const float contentHeight = controlHeight - headerHeight - contentPadding * 2.0f;
	const float contentWidth = controlWidth - contentPadding * 2.0f;

	UITabContent result = fplZeroInit;
	result.area.x = controlX + contentPadding;
	result.area.y = controlY + contentPadding;
	result.area.w = contentWidth;
	result.area.h = contentHeight;
	result.activeTab = tabControl->activeTab;

	return result;
}

//
// Checkbox
//

extern bool UICheckbox(UIContext *ctx, UICheckboxData *checkbox, const float x, const float y, const char *label, const bool showLabel, const bool isChecked, const bool isEnabled) {
	bool allowInput = UI__InitializeControl(ctx, &checkbox->base, checkbox, label);

	const UIBorderStyle *buttonBorderStyle = &ctx->style->buttonBorder;
	const UIControlStyle *controlStyle = &ctx->style->face;
	const UICheckboxStyle *checkboxStyle = &ctx->style->checkbox;

	const float lineHeight = UIGetLineHeight(ctx);
	const float fontHeight = UIGetFontHeight(ctx);

	TextureID fontTextureId = ctx->font.currentFontTextureID;
	fplAssert(fontTextureId != 0);

	const LoadedFont *font = ctx->font.currentFont;
	fplAssert(font != fpl_null);

	const float borderThickness = 1.5f;

	const float boxHeight = ctx->font.lineHeight;
	const float boxWidth = boxHeight;

	const float checkScale = 0.6f;
	const float checkWidth = boxWidth * checkScale;
	const float checkHeight = boxHeight * checkScale;
	const float checkX = x + (boxWidth - checkWidth) * 0.5f;
	const float checkY = y + (boxHeight - checkHeight) * 0.5f;
	
	const float labelSpacing = 4.0f;
	const float labelX = x + boxWidth + labelSpacing;

	float totalWidth = boxWidth;

	UIRectangle rect = MakeRectangle(x, y, boxWidth, boxHeight);

	bool wasPressed = allowInput && UI__MouseInteraction(ctx, &checkbox->mouseInteraction, &rect, isEnabled);

	bool isHover = checkbox->mouseInteraction.isHover;

	Color4f foregroundColor;
	if (isHover && isEnabled) {
		foregroundColor = controlStyle->foreground.hover;
	} else {
		if (isEnabled) {
			foregroundColor = controlStyle->foreground.normal;
		} else {
			foregroundColor = controlStyle->foreground.disabled;
		}
	}

	if (isEnabled) {
		if (isHover) {
			RendererDrawFilledQuad(x, y, boxWidth, boxHeight, controlStyle->background.hover);
		} else {
			RendererDrawFilledQuad(x, y, boxWidth, boxHeight, controlStyle->background.normal);
		}
	} else {
		RendererDrawFilledQuad(x, y, boxWidth, boxHeight, controlStyle->background.disabled);
	}

	RendererDrawControlBorder(x, y, boxWidth, boxHeight, borderThickness, &buttonBorderStyle->bright, &buttonBorderStyle->dark, &buttonBorderStyle->darkest, true);

	if (isChecked) {
		RendererDrawFilledQuad(checkX, checkY, checkWidth, checkHeight, checkboxStyle->checked);
	} else {
		RendererDrawFilledQuad(checkX, checkY, checkWidth, checkHeight, checkboxStyle->notChecked);
	}

	if (showLabel) {
		size_t labelLen = fplGetStringLength(label);
		if (labelLen > 0) {
			Vec2f labelSize = FontGetTextSize(font, label, labelLen, fontHeight);
			const float labelY = y + (boxHeight - labelSize.h) * 0.5f;

			RendererDrawString(font, fontTextureId, label, labelLen, labelX, labelY, fontHeight, foregroundColor);

#if DEBUG_RENDER_FONT_LINES
			const float descent = font->info.descent * fontHeight;
			const float ascent = font->info.ascent * fontHeight;
			const Color4f lineColorDescent = fplStructInit(Color4f, 1.0f, 0.0f, 0.0f, 1.0f);
			const Color4f lineColorAscent = fplStructInit(Color4f, 0.0f, 1.0f, 0.0f, 1.0f);
			RendererDrawLine(labelX, labelY + ascent, labelX + labelSize.w, labelY + ascent, 1.0f, lineColorAscent);
			RendererDrawLine(labelX, labelY - descent, labelX + labelSize.w, labelY - descent, 1.0f, lineColorDescent);
#endif

			totalWidth += labelSpacing + labelSize.w;
		}
	}

	checkbox->currentWidth = totalWidth;

	return wasPressed;
}

//
// Dialog
//

extern void UIEndDialog(UIContext *ctx, UIDialogData *dialog) {
	fplAssert(ctx != fpl_null && dialog != fpl_null);

	UIWindowNode *node = UI__FindWindowNode(ctx, dialog->window.base.id);
	fplAssert(node != fpl_null);
	UI__RemoveWindowNode(&ctx->windowList, node);
}

extern bool UIBeginDialog(UIContext *ctx, UIDialogData *dialog, const float w, const float h, const char *name) {
	fplAssert(ctx != fpl_null && dialog != fpl_null);

	bool allowInput = UI__InitializeControl(ctx, &dialog->base, dialog, name);

	if (!dialog->isShown) {
		if (dialog->isActive) {
			ctx->activeWindowId = dialog->base.parentId;
			dialog->isActive = false;
		}
		return false;
	}

	const UIIdentifier id = dialog->base.id;

	UIWindowNode *node = UI__PushWindow(ctx, id, w, h);
	if (node == fpl_null) {
		return false;
	}

	UIWindowNode *parent = node->next;
	fplAssert(parent != fpl_null);

	UIWindowData parentWindow = parent->data;

	dialog->window = node->data;

	float x = dialog->window.pos.x;
	float y = dialog->window.pos.y;

	if (!dialog->isActive) {
		ctx->activeWindowId = dialog->base.id;
		dialog->isActive = true;
	}

	// Draw full gray over parent
	Color4f backColor = { 0.0f, 0.0f, 0.0f, 0.5f };
	RendererDrawFilledQuad(parentWindow.pos.x, parentWindow.pos.y, parentWindow.pos.w, parentWindow.pos.h, backColor);

	// Draw panel
	UIPanel(ctx, x, y, w, h, false);

	return true;
}