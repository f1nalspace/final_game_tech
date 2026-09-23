/*
Name:
	FPL_ImageViewer | View transform

Description:
	Places a picture into the viewport: scale, whole pixel origin and the visible part of the picture.
	Viewport coordinates are pixels with the origin at the top-left corner and y pointing down, the same convention as mouse positions and final_ui.
	Picture coordinates are picture pixels with the same orientation, pixel centers are at +0.5.
	Pure math without GL, so --selftest checks it without a window.

Usage:
	Define VIEW_TRANSFORM_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef VIEW_TRANSFORM_H
#define VIEW_TRANSFORM_H

#include <stdint.h>

typedef enum ViewZoomMode {
	// Fit into the viewport when the picture is larger than it, otherwise 1:1
	ViewZoomMode_ShrinkToFit = 0,
	// Always fit into the viewport, small pictures are upscaled
	ViewZoomMode_Fit,
	// One picture pixel is one viewport pixel
	ViewZoomMode_ActualSize,
	// Fixed scale from ViewState.customScale
	ViewZoomMode_Custom,
} ViewZoomMode;

typedef struct ViewState {
	ViewZoomMode zoomMode;
	// Viewport pixels per picture pixel, used by ViewZoomMode_Custom only
	float customScale;
} ViewState;

typedef struct ViewSize {
	uint32_t width;
	uint32_t height;
} ViewSize;

typedef struct ViewRect {
	float left;
	float top;
	float width;
	float height;
} ViewRect;

typedef struct ViewTransform {
	// Viewport pixels per picture pixel as the zoom mode asks for it
	float scale;
	// Viewport pixels per picture pixel on each axis, as displayed: the displayed size is rounded to whole pixels, so they differ from scale by less than one pixel across the picture
	float scaleX;
	float scaleY;
	// The picture in viewport pixels, all whole pixels, so 1:1 is exact, the picture edges are sharp and moving the picture never shimmers
	ViewRect imageRect;
	// The part of the picture inside the viewport, in picture pixels
	ViewRect visibleSourceRect;
} ViewTransform;

// Maps a displayed picture pixel (u, v) to the stored pixel: stored = origin + u * stepU + v * stepV.
// Orientations are the EXIF values 1..8, the pixels themselves are never copied.
typedef struct ViewOrientationMapping {
	int32_t originX;
	int32_t originY;
	int32_t stepUX;
	int32_t stepUY;
	int32_t stepVX;
	int32_t stepVY;
} ViewOrientationMapping;

// Size of the picture as displayed: width and height swap for the orientations that turn by 90 degrees (5 to 8)
extern ViewSize ComputeViewOrientedSize(const uint32_t orientation, const ViewSize storedSize);
// Mapping for an EXIF orientation (1..8, anything else is 1) of a picture with this stored size
extern ViewOrientationMapping ComputeViewOrientationMapping(const uint32_t orientation, const ViewSize storedSize);

// Largest scale that fits the whole picture into the viewport, 1 for an empty picture or viewport
extern float ComputeViewFitScale(const ViewSize pictureSize, const ViewSize viewportSize);

// Scale and placement of the picture for the view state, the picture is centered on both axes
extern ViewTransform ComputeViewTransform(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize);

#endif // VIEW_TRANSFORM_H

#if defined(VIEW_TRANSFORM_IMPLEMENTATION) && !defined(VIEW_TRANSFORM_IMPLEMENTED)
#define VIEW_TRANSFORM_IMPLEMENTED

#include <math.h>

static const float ViewDefaultScale = 1.0f;
static const float ViewMinimumDisplayedLength = 1.0f;

// EXIF orientation values
static const uint32_t ViewOrientationNormal = 1;
static const uint32_t ViewOrientationMirrorHorizontal = 2;
static const uint32_t ViewOrientationRotate180 = 3;
static const uint32_t ViewOrientationMirrorVertical = 4;
static const uint32_t ViewOrientationTranspose = 5;
static const uint32_t ViewOrientationRotate90 = 6;
static const uint32_t ViewOrientationTransverse = 7;
static const uint32_t ViewOrientationRotate270 = 8;

static bool ViewIsOrientationTurned(const uint32_t orientation) {
	bool result = orientation >= ViewOrientationTranspose && orientation <= ViewOrientationRotate270;
	return(result);
}

extern ViewSize ComputeViewOrientedSize(const uint32_t orientation, const ViewSize storedSize) {
	ViewSize result = storedSize;
	if (ViewIsOrientationTurned(orientation)) {
		result.width = storedSize.height;
		result.height = storedSize.width;
	}
	return(result);
}

static ViewOrientationMapping ViewMakeMapping(const int32_t originX, const int32_t originY, const int32_t stepUX, const int32_t stepUY, const int32_t stepVX, const int32_t stepVY) {
	ViewOrientationMapping result;
	result.originX = originX;
	result.originY = originY;
	result.stepUX = stepUX;
	result.stepUY = stepUY;
	result.stepVX = stepVX;
	result.stepVY = stepVY;
	return(result);
}

// EXIF names where row 0 and column 0 of the stored picture belong. u is the displayed column, v the displayed row.
extern ViewOrientationMapping ComputeViewOrientationMapping(const uint32_t orientation, const ViewSize storedSize) {
	int32_t lastX = (int32_t)storedSize.width - 1;
	int32_t lastY = (int32_t)storedSize.height - 1;
	if (orientation == ViewOrientationMirrorHorizontal) {
		// x = lastX - u, y = v
		return(ViewMakeMapping(lastX, 0, -1, 0, 0, 1));
	}
	if (orientation == ViewOrientationRotate180) {
		// x = lastX - u, y = lastY - v
		return(ViewMakeMapping(lastX, lastY, -1, 0, 0, -1));
	}
	if (orientation == ViewOrientationMirrorVertical) {
		// x = u, y = lastY - v
		return(ViewMakeMapping(0, lastY, 1, 0, 0, -1));
	}
	if (orientation == ViewOrientationTranspose) {
		// x = v, y = u
		return(ViewMakeMapping(0, 0, 0, 1, 1, 0));
	}
	if (orientation == ViewOrientationRotate90) {
		// x = v, y = lastY - u
		return(ViewMakeMapping(0, lastY, 0, -1, 1, 0));
	}
	if (orientation == ViewOrientationTransverse) {
		// x = lastX - v, y = lastY - u
		return(ViewMakeMapping(lastX, lastY, 0, -1, -1, 0));
	}
	if (orientation == ViewOrientationRotate270) {
		// x = lastX - v, y = u
		return(ViewMakeMapping(lastX, 0, 0, 1, -1, 0));
	}
	// Normal and every unknown value: x = u, y = v
	return(ViewMakeMapping(0, 0, 1, 0, 0, 1));
}

extern float ComputeViewFitScale(const ViewSize pictureSize, const ViewSize viewportSize) {
	if (pictureSize.width == 0 || pictureSize.height == 0 || viewportSize.width == 0 || viewportSize.height == 0) {
		return(ViewDefaultScale);
	}
	float scaleX = (float)viewportSize.width / (float)pictureSize.width;
	float scaleY = (float)viewportSize.height / (float)pictureSize.height;
	float result = scaleX < scaleY ? scaleX : scaleY;
	return(result);
}

// Offset that centers a whole pixel length inside another one, rounded down to a whole pixel
static float ViewCenterOffset(const float containerLength, const float contentLength) {
	const float half = 0.5f;
	float offset = (containerLength - contentLength) * half;
	float result = floorf(offset);
	return(result);
}

// Picture length times scale, rounded to whole pixels and at least one pixel for a non-empty picture
static float ViewDisplayedLength(const float pictureLength, const float scale) {
	if (pictureLength <= 0.0f) {
		return(0.0f);
	}
	float displayed = roundf(pictureLength * scale);
	float result = displayed < ViewMinimumDisplayedLength ? ViewMinimumDisplayedLength : displayed;
	return(result);
}

// Displayed scale of one axis, the requested scale for an empty picture
static float ViewAxisScale(const float displayedLength, const float pictureLength, const float scale) {
	float result = pictureLength > 0.0f ? displayedLength / pictureLength : scale;
	return(result);
}

static float ViewClamp(const float value, const float minimum, const float maximum) {
	float result = value < minimum ? minimum : (value > maximum ? maximum : value);
	return(result);
}

extern ViewTransform ComputeViewTransform(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize) {
	float fitScale = ComputeViewFitScale(pictureSize, viewportSize);
	float scale;
	switch (viewState->zoomMode) {
		case ViewZoomMode_Fit:
			scale = fitScale;
			break;
		case ViewZoomMode_ActualSize:
			scale = ViewDefaultScale;
			break;
		case ViewZoomMode_Custom:
			scale = viewState->customScale > 0.0f ? viewState->customScale : ViewDefaultScale;
			break;
		case ViewZoomMode_ShrinkToFit:
		default:
			scale = fitScale < ViewDefaultScale ? fitScale : ViewDefaultScale;
			break;
	}

	float pictureWidth = (float)pictureSize.width;
	float pictureHeight = (float)pictureSize.height;
	float viewportWidth = (float)viewportSize.width;
	float viewportHeight = (float)viewportSize.height;
	float displayedWidth = ViewDisplayedLength(pictureWidth, scale);
	float displayedHeight = ViewDisplayedLength(pictureHeight, scale);

	ViewTransform result;
	result.scale = scale;
	result.scaleX = ViewAxisScale(displayedWidth, pictureWidth, scale);
	result.scaleY = ViewAxisScale(displayedHeight, pictureHeight, scale);
	result.imageRect.left = ViewCenterOffset(viewportWidth, displayedWidth);
	result.imageRect.top = ViewCenterOffset(viewportHeight, displayedHeight);
	result.imageRect.width = displayedWidth;
	result.imageRect.height = displayedHeight;

	// Viewport edges mapped back into the picture and clamped to it
	float sourceLeft = ViewClamp(-result.imageRect.left / result.scaleX, 0.0f, pictureWidth);
	float sourceTop = ViewClamp(-result.imageRect.top / result.scaleY, 0.0f, pictureHeight);
	float sourceRight = ViewClamp((viewportWidth - result.imageRect.left) / result.scaleX, 0.0f, pictureWidth);
	float sourceBottom = ViewClamp((viewportHeight - result.imageRect.top) / result.scaleY, 0.0f, pictureHeight);
	result.visibleSourceRect.left = sourceLeft;
	result.visibleSourceRect.top = sourceTop;
	result.visibleSourceRect.width = sourceRight - sourceLeft;
	result.visibleSourceRect.height = sourceBottom - sourceTop;
	return(result);
}

#endif // VIEW_TRANSFORM_IMPLEMENTATION
