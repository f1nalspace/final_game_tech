/*
-------------------------------------------------------------------------------
Name:
	FTT | Minimal immediate-mode UI

Description:
	A tiny immediate-mode GUI for legacy/fixed-function OpenGL: labels,
	buttons, checkboxes and radio buttons, stacked vertically inside a
	panel. Meant to be drawn in a y-down pixel-space ortho
	(glOrtho(0, width, height, 0, -1, 1)) so widget rects line up directly
	with mouse coordinates from the platform layer.

	Include this header after final_dynamic_opengl.h and ftt_linefont.h.

Requirements:
	- C17 Compiler
	- Legacy/fixed-function OpenGL (glBegin/glEnd)

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#ifndef FTT_IMMUI_H
#define FTT_IMMUI_H

#include <stdbool.h>

#include "linefont.h"

enum {
	UiPanelPaddingPixels = 10,
	UiItemHeightPixels = 22,
	UiItemSpacingPixels = 8,
	UiGroupSpacingPixels = 14,
	UiCheckboxBoxSizePixels = 14,
	UiRadioBoxSizePixels = 14,
	UiBoxToLabelGapPixels = 8,
};

static const float UiTextScale = 1.6f;
static const float UiTextThickness = 1.25f;
static const float UiWidgetOutlineThickness = 1.0f;

typedef struct fttUiState {
	float mouseX, mouseY;
	bool mouseDown;
	bool mouseDownPrevFrame;
	bool mousePressedThisFrame;

	float panelX, panelY, panelWidth;
	float cursorY;
} fttUiState;

static void UiBeginFrame(fttUiState *ui, float mouseX, float mouseY, bool mouseDown) {
	ui->mousePressedThisFrame = mouseDown && !ui->mouseDownPrevFrame;
	ui->mouseDown = mouseDown;
	ui->mouseDownPrevFrame = mouseDown;
	ui->mouseX = mouseX;
	ui->mouseY = mouseY;
}

static bool UiPointInRect(float pointX, float pointY, float rectX, float rectY, float rectWidth, float rectHeight) {
	return pointX >= rectX && pointX <= rectX + rectWidth && pointY >= rectY && pointY <= rectY + rectHeight;
}

static void UiDrawRectFilled(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

static void UiDrawRectOutline(float x, float y, float width, float height, float thickness) {
	glLineWidth(thickness);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

static void UiBeginPanel(fttUiState *ui, float x, float y, float width, float height) {
	ui->panelX = x;
	ui->panelY = y;
	ui->panelWidth = width;
	ui->cursorY = y + UiPanelPaddingPixels;

	glColor3f(0.13f, 0.13f, 0.16f);
	UiDrawRectFilled(x, y, width, height);
	glColor3f(0.4f, 0.4f, 0.46f);
	UiDrawRectOutline(x, y, width, height, UiWidgetOutlineThickness);
}

static void UiLabel(fttUiState *ui, const char *text) {
	float textX = ui->panelX + UiPanelPaddingPixels;
	glColor3f(0.85f, 0.85f, 0.92f);
	DrawLineTextZ(text, textX, ui->cursorY, UiTextScale, UiTextThickness);
	ui->cursorY += UiItemHeightPixels + UiItemSpacingPixels;
}

static void UiSpacer(fttUiState *ui) {
	ui->cursorY += UiGroupSpacingPixels;
}

static bool UiButton(fttUiState *ui, const char *text) {
	float x = ui->panelX + UiPanelPaddingPixels;
	float y = ui->cursorY;
	float width = ui->panelWidth - UiPanelPaddingPixels * 2.0f;
	float height = (float)UiItemHeightPixels;

	bool isHot = UiPointInRect(ui->mouseX, ui->mouseY, x, y, width, height);
	bool wasClicked = isHot && ui->mousePressedThisFrame;

	if (isHot) {
		glColor3f(0.3f, 0.3f, 0.38f);
	} else {
		glColor3f(0.22f, 0.22f, 0.28f);
	}
	UiDrawRectFilled(x, y, width, height);
	glColor3f(0.55f, 0.55f, 0.62f);
	UiDrawRectOutline(x, y, width, height, UiWidgetOutlineThickness);

	float textX = x + UiBoxToLabelGapPixels;
	float textY = y + (height - (float)FontGridRows * UiTextScale) * 0.5f;
	glColor3f(0.92f, 0.92f, 0.97f);
	DrawLineTextZ(text, textX, textY, UiTextScale, UiTextThickness);

	ui->cursorY += height + UiItemSpacingPixels;
	return wasClicked;
}

static bool UiCheckbox(fttUiState *ui, const char *text, bool *value) {
	float boxX = ui->panelX + UiPanelPaddingPixels;
	float boxY = ui->cursorY + ((float)UiItemHeightPixels - (float)UiCheckboxBoxSizePixels) * 0.5f;
	float boxSize = (float)UiCheckboxBoxSizePixels;

	float rowY = ui->cursorY;
	float rowHeight = (float)UiItemHeightPixels;
	float rowWidth = ui->panelWidth - UiPanelPaddingPixels * 2.0f;
	bool isHot = UiPointInRect(ui->mouseX, ui->mouseY, ui->panelX + UiPanelPaddingPixels, rowY, rowWidth, rowHeight);
	if (isHot && ui->mousePressedThisFrame) {
		*value = !(*value);
	}

	glColor3f(0.22f, 0.22f, 0.28f);
	UiDrawRectFilled(boxX, boxY, boxSize, boxSize);
	glColor3f(0.55f, 0.55f, 0.62f);
	UiDrawRectOutline(boxX, boxY, boxSize, boxSize, UiWidgetOutlineThickness);
	if (*value) {
		glColor3f(0.85f, 0.85f, 0.3f);
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glVertex2f(boxX + 2.0f, boxY + 2.0f);
		glVertex2f(boxX + boxSize - 2.0f, boxY + boxSize - 2.0f);
		glVertex2f(boxX + boxSize - 2.0f, boxY + 2.0f);
		glVertex2f(boxX + 2.0f, boxY + boxSize - 2.0f);
		glEnd();
	}

	float textX = boxX + boxSize + UiBoxToLabelGapPixels;
	float textY = ui->cursorY + (rowHeight - (float)FontGridRows * UiTextScale) * 0.5f;
	glColor3f(0.85f, 0.85f, 0.92f);
	DrawLineTextZ(text, textX, textY, UiTextScale, UiTextThickness);

	ui->cursorY += rowHeight + UiItemSpacingPixels;
	return *value;
}

static bool UiRadioButton(fttUiState *ui, const char *text, int *selected, int optionValue) {
	float boxX = ui->panelX + UiPanelPaddingPixels;
	float boxY = ui->cursorY + ((float)UiItemHeightPixels - (float)UiRadioBoxSizePixels) * 0.5f;
	float boxSize = (float)UiRadioBoxSizePixels;

	float rowY = ui->cursorY;
	float rowHeight = (float)UiItemHeightPixels;
	float rowWidth = ui->panelWidth - UiPanelPaddingPixels * 2.0f;
	bool isHot = UiPointInRect(ui->mouseX, ui->mouseY, ui->panelX + UiPanelPaddingPixels, rowY, rowWidth, rowHeight);
	if (isHot && ui->mousePressedThisFrame) {
		*selected = optionValue;
	}

	bool isSelected = (*selected == optionValue);

	glColor3f(0.22f, 0.22f, 0.28f);
	UiDrawRectFilled(boxX, boxY, boxSize, boxSize);
	glColor3f(0.55f, 0.55f, 0.62f);
	UiDrawRectOutline(boxX, boxY, boxSize, boxSize, UiWidgetOutlineThickness);
	if (isSelected) {
		float dotMargin = 3.0f;
		glColor3f(0.3f, 0.85f, 0.4f);
		UiDrawRectFilled(boxX + dotMargin, boxY + dotMargin, boxSize - dotMargin * 2.0f, boxSize - dotMargin * 2.0f);
	}

	float textX = boxX + boxSize + UiBoxToLabelGapPixels;
	float textY = ui->cursorY + (rowHeight - (float)FontGridRows * UiTextScale) * 0.5f;
	glColor3f(0.85f, 0.85f, 0.92f);
	DrawLineTextZ(text, textX, textY, UiTextScale, UiTextThickness);

	ui->cursorY += rowHeight + UiItemSpacingPixels;
	return isSelected;
}

#endif // FTT_IMMUI_H
