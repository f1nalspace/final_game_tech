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

//
// Relief and bevel box
//

// Matches what final_ui.h insets its own bevel by, so a box built here and a box built by the library
// carry the same chamfer
static const float UIBevelInsetSteps = 1.0f;

static fuiColor UIFadeColor(const fuiColor color, const float opacity) {
	fuiColor result = fuiColorWithAlpha(color, color.a * opacity);
	return result;
}

void UIBevelBox(fuiContext *ui, const fuiRect rect, const fuiColor faceColor, const UIRelief relief, const float opacity) {
	fuiTheme *theme = fuiGetTheme(ui);

	if (relief == UIRelief_Flat) {
		fuiDrawRect(ui, rect, UIFadeColor(faceColor, opacity));
		fuiDrawRectOutline(ui, rect, UIFadeColor(theme->panelBorderColor, opacity), theme->widgetBorderThickness);
		return;
	}

	// The light comes from overhead, so a raised face catches it along its top and a sunken one lies in
	// its own shadow there
	const bool isSunken = (relief == UIRelief_Sunken);
	const float shadingStrength = theme->widgetFaceShadingStrength;
	const fuiColor litFace = fuiColorShade(faceColor, shadingStrength);
	const fuiColor shadedFace = fuiColorShade(faceColor, -shadingStrength);
	const fuiColor topFace = isSunken ? shadedFace : litFace;
	const fuiColor bottomFace = isSunken ? litFace : shadedFace;
	fuiDrawRectVerticalGradient(ui, rect, UIFadeColor(topFace, opacity), UIFadeColor(bottomFace, opacity));

	// Inside the outline rather than under it, so the frame stays the outermost thing on the box
	const fuiRect bevelBox = fuiRectInflate(rect, -theme->widgetBorderThickness * UIBevelInsetSteps);
	const fuiColor topLeftEdgeColor = isSunken ? theme->widgetBevelShadowColor : theme->widgetBevelLightColor;
	const fuiColor bottomRightEdgeColor = isSunken ? theme->widgetBevelLightColor : theme->widgetBevelShadowColor;

	const float smallestSide = fuiMinF(bevelBox.w, bevelBox.h);
	const float maximumThickness = smallestSide * 0.5f;
	const float edgeThickness = fuiMinF(theme->widgetBevelThickness, maximumThickness);
	if (edgeThickness > 0.0f) {
		const float innerHeight = bevelBox.h - edgeThickness * 2.0f;
		const fuiRect topEdge = fuiRectMake(bevelBox.x, bevelBox.y, bevelBox.w, edgeThickness);
		const fuiRect leftEdge = fuiRectMake(bevelBox.x, bevelBox.y + edgeThickness, edgeThickness, innerHeight);
		const fuiRect bottomEdge = fuiRectMake(bevelBox.x, bevelBox.y + bevelBox.h - edgeThickness, bevelBox.w, edgeThickness);
		const fuiRect rightEdge = fuiRectMake(bevelBox.x + bevelBox.w - edgeThickness, bevelBox.y + edgeThickness, edgeThickness, innerHeight);

		fuiDrawRect(ui, topEdge, UIFadeColor(topLeftEdgeColor, opacity));
		fuiDrawRect(ui, leftEdge, UIFadeColor(topLeftEdgeColor, opacity));
		fuiDrawRect(ui, bottomEdge, UIFadeColor(bottomRightEdgeColor, opacity));
		fuiDrawRect(ui, rightEdge, UIFadeColor(bottomRightEdgeColor, opacity));
	}

	fuiDrawRectOutline(ui, rect, UIFadeColor(theme->panelBorderColor, opacity), theme->widgetBorderThickness);
}

//
// Tab strip
//

// Thickness of the accent line drawn under the header whose page is showing, matching the library's own tabs
static const float UITabAccentThickness = 3.0f;

// Gap between two headers, matching the library's own tabs
static const float UITabHeaderSpacing = 2.0f;

static fuiColor UIWidgetFillColor(fuiContext *ui, const fuiInteraction interaction) {
	fuiTheme *theme = fuiGetTheme(ui);
	if (interaction.isHeld) {
		return theme->widgetActiveColor;
	}
	if (interaction.isHovered) {
		return theme->widgetHoveredColor;
	}
	return theme->widgetColor;
}

int32_t UITabStrip(fuiContext *ui, const fuiRect rect, const char *id, const char *const *tabs, const int32_t tabCount, int32_t *selectedIndex) {
	if (ui == fpl_null || id == fpl_null || tabs == fpl_null || tabCount <= 0 || selectedIndex == fpl_null) {
		return 0;
	}

	fuiTheme *theme = fuiGetTheme(ui);

	int32_t activeTab = (int32_t)fuiClampF((float)*selectedIndex, 0.0f, (float)(tabCount - 1));

	fuiPushId(ui, id);
	float headerLeft = rect.x;
	for (int32_t tabIndex = 0; tabIndex < tabCount; ++tabIndex) {
		fuiVec2 labelSize = fuiMeasureText(ui, tabs[tabIndex], 0, theme->fontHeight);
		float headerWidth = labelSize.x + theme->widgetPaddingX * 2.0f;
		fuiRect headerRect = fuiRectMake(headerLeft, rect.y, headerWidth, rect.h);

		fuiPushIdInt(ui, tabIndex);
		fuiId headerId = fuiGetId(ui, "__tabStripHeader");
		fuiPopId(ui);
		fuiInteraction interaction = fuiInteract(ui, headerId, headerRect);

		bool isShowing = (tabIndex == activeTab);
		fuiColor fill = isShowing ? theme->widgetActiveColor : UIWidgetFillColor(ui, interaction);

		// The tab in front stands out of the strip and the ones behind it lie flat in it, the same three
		// reliefs the library's own tab control uses
		UIRelief relief = UIRelief_Flat;
		if (isShowing) {
			relief = UIRelief_Raised;
		} else if (interaction.isHeld) {
			relief = UIRelief_Sunken;
		}
		UIBevelBox(ui, headerRect, fill, relief, 1.0f);

		float labelX = headerRect.x + (headerRect.w - labelSize.x) * 0.5f;
		float labelY = headerRect.y + (headerRect.h - labelSize.y) * 0.5f;
		fuiDrawText(ui, tabs[tabIndex], 0, fuiV2(labelX, labelY), theme->fontHeight, theme->textColor);

		if (isShowing) {
			fuiRect underline = fuiRectMake(headerRect.x, headerRect.y + headerRect.h - UITabAccentThickness, headerRect.w, UITabAccentThickness);
			fuiDrawRect(ui, underline, theme->accentColor);
		}

		if (interaction.wasClicked) {
			activeTab = tabIndex;
		}

		headerLeft += headerWidth + UITabHeaderSpacing;
	}
	fuiPopId(ui);

	*selectedIndex = activeTab;
	return activeTab;
}

//
// Text view
//

// How many lines of one block of text the view can lay out. Text longer than this is cut, which is why the
// content the about dialog carries is kept well under it
#define UI_TEXT_VIEW_MAX_LINES 512

// How many rows one notch of the mouse wheel travels
static const float UITextViewWheelRows = 3.0f;

// How far one notch of the wheel pans sideways while shift is held, in pixels
static const float UITextViewWheelPanX = 48.0f;

// Space between the text and the edges of the box it sits in
static const float UITextViewPadding = 6.0f;

// Broken out of the text once per call rather than kept, because only one view is built at a time and the
// lines point straight into the caller's own string
static fuiTextLine UITextViewLines[UI_TEXT_VIEW_MAX_LINES];

// The horizontal twin of fuiScrollbarVertical, which the library only offers along the other axis
static float UIScrollbarHorizontal(fuiContext *ui, const fuiRect track, const char *id, const float scroll, const float viewportLength, const float contentLength) {
	fuiTheme *theme = fuiGetTheme(ui);

	// The same minimum the library keeps, so a thumb never shrinks past being grabbable
	const float minimumThumbWidth = 18.0f;
	const float disabledThumbAlpha = 0.35f;

	const float maximumScroll = contentLength - viewportLength;
	if (maximumScroll <= 0.0f) {
		// Drawn but disabled, rather than hidden, so the box does not change shape the moment its content grows
		fuiColor dimmedThumb = fuiColorWithAlpha(theme->widgetColor, theme->widgetColor.a * disabledThumbAlpha);
		fuiDrawRect(ui, track, theme->widgetTrackColor);
		fuiDrawRect(ui, track, dimmedThumb);
		fuiDrawRectOutline(ui, track, theme->panelBorderColor, theme->widgetBorderThickness);
		return 0.0f;
	}

	const float visibleRatio = (contentLength > 0.0f) ? (viewportLength / contentLength) : 1.0f;
	const float thumbWidth = fuiMaxF(track.w * visibleRatio, minimumThumbWidth);
	const float trackRange = track.w - thumbWidth;

	const float scrollRatioBeforeDrag = (maximumScroll > 0.0f) ? (scroll / maximumScroll) : 0.0f;
	const fuiRect thumbBeforeDrag = fuiRectMake(track.x + scrollRatioBeforeDrag * trackRange, track.y, thumbWidth, track.h);

	fuiId thumbId = fuiGetId(ui, id);
	fuiInteraction interaction = fuiInteract(ui, thumbId, thumbBeforeDrag);

	float newScroll = scroll;
	if (interaction.isHeld && trackRange > 0.0f) {
		float scrollPerPixel = maximumScroll / trackRange;
		fuiVec2 mouseDelta = fuiGetMouseDelta(ui);
		newScroll += mouseDelta.x * scrollPerPixel;
	}
	newScroll = fuiClampF(newScroll, 0.0f, maximumScroll);

	const float scrollRatio = newScroll / maximumScroll;
	const fuiRect thumb = fuiRectMake(track.x + scrollRatio * trackRange, track.y, thumbWidth, track.h);
	const fuiColor thumbColor = interaction.isHeld ? theme->accentColor : (interaction.isHovered ? theme->widgetHoveredColor : theme->widgetColor);

	fuiDrawRect(ui, track, theme->widgetTrackColor);
	fuiDrawRect(ui, thumb, thumbColor);
	fuiDrawRectOutline(ui, thumb, theme->panelBorderColor, theme->widgetBorderThickness);

	return newScroll;
}

// Measures the widest line and the stacked height of the whole block, but only when the block or the size
// it is drawn at has actually changed since the last time round
static void UITextViewMeasure(fuiContext *ui, const char *text, const float fontHeight, const float lineHeight, UITextViewState *state) {
	if (state->measuredText == text && state->measuredFontHeight == fontHeight) {
		return;
	}

	const bool noWordWrap = false;
	const float unusedWrapWidth = 0.0f;
	uint32_t lineCount = fuiBreakTextLines(ui, text, 0, fontHeight, noWordWrap, unusedWrapWidth, UITextViewLines, UI_TEXT_VIEW_MAX_LINES);

	float widestLine = 0.0f;
	for (uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		const fuiTextLine *line = UITextViewLines + lineIndex;
		fuiVec2 lineSize = fuiMeasureText(ui, line->start, line->length, fontHeight);
		widestLine = fuiMaxF(widestLine, lineSize.x);
	}

	state->measuredText = text;
	state->measuredFontHeight = fontHeight;
	state->contentWidth = widestLine;
	state->contentHeight = (float)lineCount * lineHeight;
}

static bool UIRectContainsPoint(const fuiRect rect, const fuiVec2 point) {
	bool result = (point.x >= rect.x) && (point.x < (rect.x + rect.w)) && (point.y >= rect.y) && (point.y < (rect.y + rect.h));
	return result;
}

void UITextView(fuiContext *ui, const fuiRect rect, const char *id, const char *text, UITextViewState *state, const float backgroundOpacity) {
	if (ui == fpl_null || id == fpl_null || text == fpl_null || state == fpl_null) {
		return;
	}

	fuiTheme *theme = fuiGetTheme(ui);

	const float fontHeight = theme->fontHeight;
	const float lineHeight = fuiGetLineHeight(ui, fontHeight);
	const float gutter = fuiScrollGutterWidth();

	// The frame is drawn whatever the fill does, so a view with no fill at all is still a box rather than
	// loose text floating over whatever lies behind it
	const float clampedBackgroundOpacity = fuiClampF(backgroundOpacity, 0.0f, 1.0f);
	if (clampedBackgroundOpacity > 0.0f) {
		fuiDrawRect(ui, rect, UIFadeColor(theme->widgetTrackColor, clampedBackgroundOpacity));
	}
	fuiDrawRectOutline(ui, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	// Both gutters are always reserved, so the text does not shift the moment a scrollbar becomes live
	const fuiRect viewport = fuiRectMake(rect.x, rect.y, rect.w - gutter, rect.h - gutter);

	UITextViewMeasure(ui, text, fontHeight, lineHeight, state);

	const float visibleWidth = fuiMaxF(0.0f, viewport.w - UITextViewPadding * 2.0f);
	const float visibleHeight = fuiMaxF(0.0f, viewport.h - UITextViewPadding * 2.0f);
	const float maximumScrollX = fuiMaxF(0.0f, state->contentWidth - visibleWidth);
	const float maximumScrollY = fuiMaxF(0.0f, state->contentHeight - visibleHeight);

	float scrollX = state->scrollX;
	float scrollY = state->scrollY;

	// The wheel scrolls whichever axis the shift key asks for, which is the only way sideways scrolling is
	// reachable without letting go of the wheel and grabbing the bar
	if (UIRectContainsPoint(rect, fuiGetMousePosition(ui))) {
		float wheelDelta = fuiGetMouseWheelDelta(ui);
		if (wheelDelta != 0.0f) {
			if (fuiIsShiftDown(ui)) {
				scrollX -= wheelDelta * UITextViewWheelPanX;
			} else {
				scrollY -= wheelDelta * lineHeight * UITextViewWheelRows;
			}
		}
	}

	fuiPushId(ui, id);

	// Resolved before the text, so it is laid out from the offset the bars settled on
	const fuiRect verticalTrack = fuiRectMake(rect.x + rect.w - gutter, rect.y, gutter, rect.h - gutter);
	scrollY = fuiScrollbarVertical(ui, verticalTrack, "__textViewScrollbarY", scrollY, visibleHeight, state->contentHeight);
	scrollY = fuiClampF(scrollY, 0.0f, maximumScrollY);

	const fuiRect horizontalTrack = fuiRectMake(rect.x, rect.y + rect.h - gutter, rect.w - gutter, gutter);
	scrollX = UIScrollbarHorizontal(ui, horizontalTrack, "__textViewScrollbarX", scrollX, visibleWidth, state->contentWidth);
	scrollX = fuiClampF(scrollX, 0.0f, maximumScrollX);

	// The little square where the two bars meet, filled so it does not read as a hole in the box
	const fuiRect scrollCorner = fuiRectMake(rect.x + rect.w - gutter, rect.y + rect.h - gutter, gutter, gutter);
	fuiDrawRect(ui, scrollCorner, theme->widgetTrackColor);

	state->scrollX = scrollX;
	state->scrollY = scrollY;

	const bool noWordWrap = false;
	const float unusedWrapWidth = 0.0f;
	uint32_t lineCount = fuiBreakTextLines(ui, text, 0, fontHeight, noWordWrap, unusedWrapWidth, UITextViewLines, UI_TEXT_VIEW_MAX_LINES);

	const float textX = viewport.x + UITextViewPadding - scrollX;
	const float firstLineY = viewport.y + UITextViewPadding - scrollY;

	fuiPushClip(ui, viewport);
	for (uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		const fuiTextLine *line = UITextViewLines + lineIndex;
		if (line->length == 0) {
			continue;
		}

		float lineY = firstLineY + (float)lineIndex * lineHeight;
		bool isAboveTheBox = (lineY + lineHeight) < viewport.y;
		bool isBelowTheBox = lineY > (viewport.y + viewport.h);
		if (isAboveTheBox || isBelowTheBox) {
			continue;
		}

		fuiDrawText(ui, line->start, line->length, fuiV2(textX, lineY), fontHeight, theme->textColor);
	}
	fuiPopClip(ui);

	// A wheel notch over the box belongs to the box, not to whatever lies behind the dialog
	fuiBlockMouse(ui, rect);

	fuiPopId(ui);
}

//
// Icon button
//

// How far the icon sinks into its own square while it is held, as a fraction of its size. Just enough to
// read as a press without the picture appearing to jump
static const float UIIconButtonPressInsetFactor = 0.04f;

bool UIIconButton(fuiContext *ui, const fuiRect rect, const char *id, const Texture *icon, const float opacity) {
	if (ui == fpl_null || id == fpl_null) {
		return false;
	}

	fuiId buttonId = fuiGetId(ui, id);
	fuiInteraction interaction = fuiInteract(ui, buttonId, rect);

	// A hovered icon comes fully into view whatever opacity was asked for, because the cursor being on it
	// is exactly the moment it has to be readable
	const float drawOpacity = interaction.isHovered ? 1.0f : fuiClampF(opacity, 0.0f, 1.0f);

	if (icon != fpl_null && icon->isValid) {
		const float pressInset = interaction.isHeld ? (fuiMinF(rect.w, rect.h) * UIIconButtonPressInsetFactor) : 0.0f;
		const fuiRect iconBox = fuiRectInflate(rect, -pressInset);

		fuiImageDesc iconImage = fplZeroInit;
		iconImage.texture = (fuiTextureId)icon->id;
		iconImage.textureSize = fuiV2((float)icon->width, (float)icon->height);
		iconImage.uvMin = fuiV2(0.0f, 0.0f);
		iconImage.uvMax = fuiV2(icon->uScale, icon->vScale);
		iconImage.tint = fuiColorRGBA(1.0f, 1.0f, 1.0f, drawOpacity);
		iconImage.scaleMode = FUI_IMAGE_SCALE_LETTERBOX;
		// The image loader flips every texture on the way in, so it is turned back over here
		iconImage.flags = FUI_IMAGE_FLIP_V;
		fuiImage(ui, iconBox, &iconImage);
	}

	return interaction.wasClicked;
}
