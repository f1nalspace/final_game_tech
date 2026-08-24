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
// Font
//

// Body text is drawn at the theme's font height and baked at twice it, which is the ratio the previous
// renderer used and the one that reads cleanly through a linear filter
static const float UIBodyFontBakeHeight = 40.0f;
static const uint32_t UIBodyFontAtlasSize = 512;

// Watermarks are drawn at four times the body height and baked just above it, so they scale DOWN slightly
// rather than being stretched up out of the body atlas
static const float UILargeFontBakeHeight = 96.0f;
static const uint32_t UILargeFontAtlasSize = 1024;

static bool UIFontAtlasCreate(UIFontAtlas *atlas, const void *trueTypeData, const float bakePixelHeight, const uint32_t atlasSize) {
	fplMemoryClear(atlas, sizeof(*atlas));

	fuiStbttBakeSettings settings = fuiStbttDefaultBakeSettings();
	settings.pixelHeight = bakePixelHeight;
	settings.atlasWidth = atlasSize;
	settings.atlasHeight = atlasSize;

	// An atlas too small for the whole range is a failure rather than a partial bake, so this cannot
	// silently lose half the alphabet
	if (!fuiStbttFontBake(&atlas->baked, trueTypeData, &settings)) {
		return false;
	}

	// One byte of coverage per texel, which is what an alpha texture modulated by the vertex color draws text with
	atlas->atlasTexture = RendererTextureUpload(atlas->baked.atlasWidth, atlas->baked.atlasHeight, TextureFormat_Alpha, TextureFilter_Linear, atlas->baked.atlasPixels);
	if (!atlas->atlasTexture.isValid) {
		fuiStbttFontRelease(&atlas->baked);
		return false;
	}

	atlas->font = fuiStbttFontToFuiFont(&atlas->baked, (fuiTextureId)atlas->atlasTexture.id);

	return true;
}

static void UIFontAtlasRelease(UIFontAtlas *atlas) {
	RendererTextureRelease(&atlas->atlasTexture);
	fuiStbttFontRelease(&atlas->baked);
}

bool UIFontCreate(UIFont *font, const void *trueTypeData) {
	if (font == fpl_null || trueTypeData == fpl_null) {
		return false;
	}

	if (!UIFontAtlasCreate(&font->body, trueTypeData, UIBodyFontBakeHeight, UIBodyFontAtlasSize)) {
		return false;
	}

	if (!UIFontAtlasCreate(&font->large, trueTypeData, UILargeFontBakeHeight, UILargeFontAtlasSize)) {
		UIFontAtlasRelease(&font->body);
		return false;
	}

	return true;
}

void UIFontRelease(UIFont *font) {
	if (font == fpl_null) {
		return;
	}
	UIFontAtlasRelease(&font->large);
	UIFontAtlasRelease(&font->body);
}

//
// Theme
//

// Turns the 0..255 components the old theme was written in into the 0..1 the library takes
static fuiColor UIColorFromBytes(const uint8_t r, const uint8_t g, const uint8_t b, const float a) {
	const float inverse255 = 1.0f / 255.0f;
	fuiColor result = fuiColorRGBA((float)r * inverse255, (float)g * inverse255, (float)b * inverse255, a);
	return result;
}

void UIApplyDarkTheme(fuiContext *ui, const float fontHeight, const float lineHeight) {
	fuiTheme *theme = fuiGetTheme(ui);
	if (theme == fpl_null) {
		return;
	}

	// The palette the hand rolled UI used, so the port does not change how the debugger looks
	theme->panelBackgroundColor = UIColorFromBytes(31, 31, 31, 1.0f);
	theme->panelBorderColor = UIColorFromBytes(102, 102, 102, 1.0f);
	theme->textColor = UIColorFromBytes(220, 220, 204, 1.0f);
	theme->textMutedColor = UIColorFromBytes(120, 120, 104, 1.0f);
	theme->accentColor = UIColorFromBytes(240, 240, 240, 1.0f);

	theme->widgetColor = UIColorFromBytes(51, 51, 51, 1.0f);
	theme->widgetHoveredColor = UIColorFromBytes(71, 71, 71, 1.0f);
	theme->widgetActiveColor = UIColorFromBytes(90, 90, 90, 1.0f);
	theme->widgetTrackColor = UIColorFromBytes(37, 37, 38, 1.0f);
	theme->textSelectionColor = fuiColorRGBA(0.25f, 0.25f, 0.75f, 0.75f);
	// A slider draws its caption straight over the knob, so the knob has to stay dark enough for light text
	// to read across it. The near-white knob of the stock theme swallows whichever glyph it happens to cover
	theme->knobColor = UIColorFromBytes(110, 110, 110, 1.0f);
	theme->menuHighlightColor = fuiColorRGBA(0.25f, 0.25f, 0.75f, 1.0f);
	theme->modalBackdropColor = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.55f);

	theme->tooltipBackgroundColor = UIColorFromBytes(37, 37, 38, 0.97f);
	theme->tooltipBorderColor = UIColorFromBytes(102, 102, 102, 1.0f);
	theme->tooltipTextColor = UIColorFromBytes(220, 220, 204, 1.0f);

	theme->fontHeight = fontHeight;
	theme->menuItemHeight = lineHeight;
	theme->menuItemFontHeight = fontHeight;
}

//
// Platform services
//

static bool UIPlatformGetClipboardText(void *userData, char *destination, uint32_t maxDestinationLength) {
	(void)userData;
	bool result = fplGetClipboardText(destination, maxDestinationLength);
	return result;
}

static bool UIPlatformSetClipboardText(void *userData, const char *text) {
	(void)userData;
	bool result = fplSetClipboardText(text);
	return result;
}

void UIInstallPlatform(fuiContext *ui) {
	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = UIPlatformGetClipboardText;
	platform.setClipboardText = UIPlatformSetClipboardText;
	fuiSetPlatform(ui, &platform);
}

//
// Input
//

void UIInputBeginFrame(fuiInput *input) {
	if (input == fpl_null) {
		return;
	}

	// A half transition is only true for the frame it happened in, while endedDown carries over
	for (uint32_t buttonIndex = 0; buttonIndex < FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		input->mouseButtons[buttonIndex].halfTransitionCount = 0;
	}
	for (uint32_t keyIndex = 0; keyIndex < FUI_KEY_COUNT; ++keyIndex) {
		input->keys[keyIndex].halfTransitionCount = 0;
	}

	input->textInputLength = 0;
	input->mouseWheelDelta = 0.0f;
}

void UIInputSetButton(fuiButtonState *button, const bool isDown) {
	if (button == fpl_null) {
		return;
	}
	if (button->endedDown != isDown) {
		button->halfTransitionCount++;
	}
	button->endedDown = isDown;
}

fuiKey UIKeyFromPlatformKey(const fplKey key) {
	// The digits, the letters and the function keys are contiguous in both enumerations, so each of the three ranges maps with one offset
	if (key >= fplKey_0 && key <= fplKey_9) {
		return (fuiKey)(FUI_KEY_0 + (key - fplKey_0));
	}
	if (key >= fplKey_A && key <= fplKey_Z) {
		return (fuiKey)(FUI_KEY_A + (key - fplKey_A));
	}
	if (key >= fplKey_F1 && key <= fplKey_F12) {
		return (fuiKey)(FUI_KEY_F1 + (key - fplKey_F1));
	}

	switch (key) {
		case fplKey_Backspace:
			return FUI_KEY_BACKSPACE;
		case fplKey_Tab:
			return FUI_KEY_TAB;
		case fplKey_Return:
			return FUI_KEY_RETURN;
		case fplKey_Escape:
			return FUI_KEY_ESCAPE;
		case fplKey_Space:
			return FUI_KEY_SPACE;
		case fplKey_PageUp:
			return FUI_KEY_PAGE_UP;
		case fplKey_PageDown:
			return FUI_KEY_PAGE_DOWN;
		case fplKey_End:
			return FUI_KEY_END;
		case fplKey_Home:
			return FUI_KEY_HOME;
		case fplKey_Left:
			return FUI_KEY_LEFT;
		case fplKey_Up:
			return FUI_KEY_UP;
		case fplKey_Right:
			return FUI_KEY_RIGHT;
		case fplKey_Down:
			return FUI_KEY_DOWN;
		case fplKey_Insert:
			return FUI_KEY_INSERT;
		case fplKey_Delete:
			return FUI_KEY_DELETE;
		case fplKey_LeftControl:
			return FUI_KEY_LEFT_CONTROL;
		case fplKey_RightControl:
			return FUI_KEY_RIGHT_CONTROL;
		case fplKey_LeftShift:
			return FUI_KEY_LEFT_SHIFT;
		case fplKey_RightShift:
			return FUI_KEY_RIGHT_SHIFT;
		case fplKey_LeftAlt:
			return FUI_KEY_LEFT_ALT;
		case fplKey_RightAlt:
			return FUI_KEY_RIGHT_ALT;
		default:
			return FUI_KEY_NONE;
	}
}

void UIInputAddText(fuiInput *input, const uint32_t codePoint) {
	if (input == fpl_null) {
		return;
	}
	if (input->textInputLength >= (int32_t)FUI_MAX_TEXT_INPUT) {
		return;
	}
	input->textInput[input->textInputLength] = codePoint;
	input->textInputLength++;
}

//
// Immediate drawing
//

// How much larger than the body text a panel watermark is drawn
static const float UIWatermarkFontScale = 4.0f;

// How much of the panel background colour is left in the watermark, so it reads as a backdrop and not as content
static const float UIWatermarkLightness = 0.35f;

fuiColor UIColorFrom4f(const Color4f color) {
	fuiColor result = fuiColorRGBA(color.r, color.g, color.b, color.a);
	return result;
}

void UIPanel(fuiContext *ui, const fuiRect rect, const bool isSunken) {
	fuiTheme *theme = fuiGetTheme(ui);
	fuiColor fill = isSunken ? theme->widgetTrackColor : theme->panelBackgroundColor;
	fuiDrawRect(ui, rect, fill);
	fuiDrawRectOutline(ui, rect, theme->panelBorderColor, theme->panelBorderThickness);
}

void UIText(fuiContext *ui, const float x, const float y, const fuiColor color, const char *text, const size_t textLen) {
	fuiTheme *theme = fuiGetTheme(ui);
	fuiDrawText(ui, text, textLen, fuiV2(x, y), theme->fontHeight, color);
}

fuiVec2 UITextSize(fuiContext *ui, const char *text, const size_t textLen) {
	fuiTheme *theme = fuiGetTheme(ui);
	fuiVec2 result = fuiMeasureText(ui, text, textLen, theme->fontHeight);
	return result;
}

void UIWatermark(fuiContext *ui, const UIFont *font, const fuiRect rect, const char *text) {
	fuiTheme *theme = fuiGetTheme(ui);
	float watermarkHeight = theme->fontHeight * UIWatermarkFontScale;
	fuiColor watermarkColor = fuiColorLerp(theme->panelBackgroundColor, theme->textMutedColor, UIWatermarkLightness);

	// Measuring and drawing both read the context's font, so the swap has to wrap the pair of them
	fuiSetFont(ui, &font->large.font);

	fuiVec2 watermarkSize = fuiMeasureText(ui, text, 0, watermarkHeight);
	float watermarkX = rect.x + (rect.w - watermarkSize.x) * 0.5f;
	float watermarkY = rect.y + (rect.h - watermarkSize.y) * 0.5f;
	fuiPushClip(ui, rect);
	fuiDrawText(ui, text, 0, fuiV2(watermarkX, watermarkY), watermarkHeight, watermarkColor);
	fuiPopClip(ui);

	fuiSetFont(ui, &font->body.font);
}

//
// Source list
//

// How many rows one notch of the mouse wheel travels
static const float UISourceListWheelRows = 3.0f;

static bool UIPointIsInsideRect(const fuiVec2 point, const fuiRect rect) {
	bool result = (point.x >= rect.x) && (point.x < (rect.x + rect.w)) && (point.y >= rect.y) && (point.y < (rect.y + rect.h));
	return result;
}

void UISourceList(fuiContext *ui, const fuiRect rect, const char *id, const StringList *lines, UISourceListState *state, const int32_t highlightIndex, const int32_t scrollToIndex) {
	if (ui == fpl_null || id == fpl_null || state == fpl_null) {
		return;
	}

	fuiTheme *theme = fuiGetTheme(ui);

	const int32_t rowCount = (lines != fpl_null) ? (int32_t)lines->count : 0;
	const float rowHeight = theme->menuItemHeight;
	const float contentLength = (float)rowCount * rowHeight;

	fuiDrawRect(ui, rect, theme->widgetTrackColor);
	fuiDrawRectOutline(ui, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	// The gutter is always reserved so the rows do not shift the moment the list grows past its box
	const float gutterWidth = fuiScrollGutterWidth();
	const float rowWidth = rect.w - gutterWidth;

	// Text stops one padding short of the gutter, so a long line never runs up against the scrollbar
	const float textClipWidth = fuiMaxF(0.0f, rowWidth - theme->widgetPaddingX);

	const float maximumScroll = fuiMaxF(0.0f, contentLength - rect.h);

	float scroll = state->scroll;

	fuiVec2 mousePosition = fuiGetMousePosition(ui);
	if (UIPointIsInsideRect(mousePosition, rect)) {
		float wheelDelta = fuiGetMouseWheelDelta(ui);
		if (wheelDelta != 0.0f) {
			scroll -= wheelDelta * rowHeight * UISourceListWheelRows;
		}
	}

	// A scroll request wins over where the user left the list, but only on the frame it is made, so
	// dragging the thumb is not fought over on every following frame
	if (scrollToIndex >= 0 && scrollToIndex < rowCount) {
		float centeredScroll = ((float)scrollToIndex * rowHeight) - (rect.h - rowHeight) * 0.5f;
		scroll = fuiClampF(centeredScroll, 0.0f, maximumScroll);
	}

	fuiPushId(ui, id);

	// Resolved before the rows, so they are laid out from the offset the scrollbar settled on
	fuiRect scrollTrack = fuiRectMake(rect.x + rect.w - gutterWidth, rect.y, gutterWidth, rect.h);
	scroll = fuiScrollbarVertical(ui, scrollTrack, "__sourceListScrollbar", scroll, rect.h, contentLength);
	scroll = fuiClampF(scroll, 0.0f, maximumScroll);
	state->scroll = scroll;

	fuiPushClip(ui, rect);
	for (int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		float rowTop = rect.y - scroll + (float)rowIndex * rowHeight;
		bool isAboveTheBox = (rowTop + rowHeight) < rect.y;
		bool isBelowTheBox = rowTop > (rect.y + rect.h);
		if (isAboveTheBox || isBelowTheBox) {
			continue;
		}

		fuiRect rowRect = fuiRectMake(rect.x, rowTop, rowWidth, rowHeight);

		bool rowIsHighlighted = (rowIndex == highlightIndex);
		bool rowIsSelected = (rowIndex == state->selectedIndex);
		if (rowIsHighlighted) {
			fuiDrawRect(ui, rowRect, theme->menuHighlightColor);
		} else if (rowIsSelected) {
			fuiDrawRect(ui, rowRect, theme->widgetActiveColor);
		}

		const String *line = lines->entries + rowIndex;
		if (line->text != fpl_null && line->len > 0) {
			float textX = rowRect.x + theme->widgetPaddingX;
			fuiVec2 textSize = fuiMeasureText(ui, line->text, line->len, theme->fontHeight);
			float textY = rowRect.y + (rowRect.h - textSize.y) * 0.5f;
			// A line longer than the row is cut short of the gutter rather than drawn across the scrollbar
			fuiRect textClip = fuiRectMake(rowRect.x, rowRect.y, textClipWidth, rowRect.h);
			fuiPushClip(ui, textClip);
			fuiDrawText(ui, line->text, line->len, fuiV2(textX, textY), theme->fontHeight, theme->textColor);
			fuiPopClip(ui);
		}

		fuiPushIdInt(ui, rowIndex);
		bool rowIsHovered = false;
		if (fuiCell(ui, rowRect, "__sourceListRow", &rowIsHovered)) {
			state->selectedIndex = rowIndex;
		}
		fuiPopId(ui);
	}
	fuiPopClip(ui);

	fuiPopId(ui);
}

//
// Checkbox and radio with an enabled state
//

// Matches the gap final_ui.h puts between a widget's box and its label
static const float UIWidgetLabelGap = 6.0f;

// How much of the panel background is washed over a disabled widget
static const float UIDisabledWashAlpha = 0.55f;

float UICheckboxWidth(fuiContext *ui, const char *label) {
	fuiTheme *theme = fuiGetTheme(ui);
	float boxSide = theme->fontHeight;
	fuiVec2 textSize = fuiMeasureText(ui, label, 0, theme->fontHeight);
	float result = theme->widgetPaddingX + boxSide + UIWidgetLabelGap + textSize.x + theme->widgetPaddingX;
	return result;
}

float UICheckboxRowX(fuiContext *ui, const float contentX) {
	fuiTheme *theme = fuiGetTheme(ui);
	float result = contentX - theme->widgetPaddingX;
	return result;
}

static void UIDrawDisabledWash(fuiContext *ui, const fuiRect rect) {
	fuiTheme *theme = fuiGetTheme(ui);
	fuiColor wash = fuiColorWithAlpha(theme->panelBackgroundColor, UIDisabledWashAlpha);
	fuiDrawRect(ui, rect, wash);
}

bool UICheckboxEx(fuiContext *ui, const fuiRect rect, const char *label, bool *value, const bool enabled) {
	if (enabled) {
		bool result = fuiCheckbox(ui, rect, label, value);
		return result;
	}

	// Built against a copy so a click cannot change anything, and washed over so it reads as disabled.
	// The widget still swallows the press, which is what keeps a disabled control from acting as a hole.
	bool scratchValue = *value;
	fuiCheckbox(ui, rect, label, &scratchValue);
	UIDrawDisabledWash(ui, rect);
	return false;
}

bool UIRadioEx(fuiContext *ui, const fuiRect rect, const char *label, int32_t *selected, const int32_t option, const bool enabled) {
	if (enabled) {
		bool result = fuiRadio(ui, rect, label, selected, option);
		return result;
	}

	int32_t scratchSelected = *selected;
	fuiRadio(ui, rect, label, &scratchSelected, option);
	UIDrawDisabledWash(ui, rect);
	return false;
}
