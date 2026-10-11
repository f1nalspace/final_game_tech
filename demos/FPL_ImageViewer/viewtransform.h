/*
Name:
	FPL_ImageViewer | View transform

Description:
	Places a picture into the viewport: scale, whole pixel origin and the visible part of the picture.
	Zooms around a point, pans, steps through the zoom steps and carries the view over to the next picture (plan section 2.6).
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

// What a picture change keeps of the view (plan section 2.6)
typedef enum ViewPersistence {
	// Every picture starts with the start view
	ViewPersistence_Reset = 0,
	// Scale relative to the fit scale and normalized center: pictures of any size show the same part at the same displayed size
	ViewPersistence_KeepRelative,
	// Scale itself and normalized center: pictures of any size show the same part at the same picture pixels per viewport pixel
	ViewPersistence_KeepAbsolute,
	ViewPersistence_Count,
} ViewPersistence;

// Initialize with ViewMakeState(), a zero center would show the top-left corner of a picture larger than the viewport
typedef struct ViewState {
	ViewZoomMode zoomMode;
	// Viewport pixels per picture pixel, used by ViewZoomMode_Custom only
	float customScale;
	// Picture point at the viewport center, 0..1 per axis of the displayed picture, in every mode. An axis that fits into the viewport is centered anyway.
	float centerX;
	float centerY;
	// ViewZoomMode_Custom kept relative over a picture change: the scale relative to the fit scale, it replaces customScale once the picture size is known (ComputeViewResolved), 0 otherwise
	float pendingRelativeScale;
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
	// Scale that fits the whole picture into the viewport, the reference of the relative scale and the zoom limits
	float fitScale;
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

// View state in a zoom mode (customScale for ViewZoomMode_Custom only) with the picture point at the viewport center
extern ViewState ViewMakeState(const ViewZoomMode zoomMode, const float customScale, const float centerX, const float centerY);

// Scale and placement of the picture for the view state. An axis on which the picture fits into the viewport is centered,
// on a larger axis the center of the view state is placed at the viewport center, clamped so that no border shows.
extern ViewTransform ComputeViewTransform(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize);

// Zoom range of the wheel and the zoom steps: half the fit scale up to 32, but always including 1:1 and the fit scale
extern void ComputeViewScaleLimits(const float fitScale, float *outMinimum, float *outMaximum);
// Next scale of the zoom steps 1/8 ... 32 above (direction > 0) or below (direction < 0) the current scale, the fit scale is a step of its own.
// outMode receives ViewZoomMode_Fit or ViewZoomMode_ActualSize when the step is the fit scale or 1:1, ViewZoomMode_Custom otherwise.
extern float ComputeViewStepScale(const float currentScale, const float fitScale, const int direction, ViewZoomMode *outMode);
// Scale after a wheel turn of wheelDelta notches (fractions zoom smoothly), snapped to the fit scale and to 1:1 when the turn would pass over them. outMode as above.
extern float ComputeViewWheelScale(const float currentScale, const float fitScale, const float wheelDelta, ViewZoomMode *outMode);

// Changes to the zoom mode (newScale is used by ViewZoomMode_Custom only) so that the picture point under the viewport point stays there
extern ViewState ComputeViewZoomAtPoint(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize, const ViewZoomMode newMode, const float newScale, const float pointX, const float pointY);
// Moves the picture by viewport pixels, a larger picture never shows a border and a smaller one stays centered
extern ViewState ComputeViewPan(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize, const float deltaX, const float deltaY);
// Turns a pending relative scale into customScale once the picture size is known, so a later window resize keeps the scale itself
extern ViewState ComputeViewResolved(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize);
// View for the next picture. pictureSize is the current picture as displayed, or zero while it is still loading (the view is then kept as it is).
extern ViewState ComputeViewForNextPicture(const ViewState *viewState, const ViewPersistence persistence, const ViewState *startView, const ViewSize pictureSize, const ViewSize viewportSize);

// Level of detail the resample pipeline reads for a displayed scale (the larger one of both axes): the smallest level that is still at least eight times the displayed size,
// so the scale relative to that level lies in (1/16, 1/8]. levelCount counts level 0, levels before firstLevel have no texture (larger than the GPU allows).
extern uint32_t ComputeViewSourceLevel(const float scale, const uint32_t levelCount, const uint32_t firstLevel);

// Where a level lies on the displayed picture, per displayed axis (u = columns, v = rows).
// Level pixel i covers the picture pixels [i * 2^level, (i + 1) * 2^level), so a level of an odd size reaches beyond the picture with its last pixel.
// On an axis that is displayed mirrored that pixel comes first, which shifts the level by levelLength - pictureLength / 2^level.
// The coverage is the part of the first and the last displayed level pixel that lies on the picture, the resample pipeline weights them by it.
typedef struct ViewLevelPlacement {
	float offsetU;
	float offsetV;
	float firstCoverageU;
	float lastCoverageU;
	float firstCoverageV;
	float lastCoverageV;
} ViewLevelPlacement;
extern ViewLevelPlacement ComputeViewLevelPlacement(const uint32_t orientation, const ViewSize storedSize, const ViewSize storedLevelSize, const uint32_t level);

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

extern ViewState ViewMakeState(const ViewZoomMode zoomMode, const float customScale, const float centerX, const float centerY) {
	ViewState result;
	result.zoomMode = zoomMode;
	result.customScale = customScale;
	result.centerX = centerX;
	result.centerY = centerY;
	result.pendingRelativeScale = 0.0f;
	return(result);
}

// Scale the zoom mode asks for, a pending relative scale goes before customScale
static float ViewModeScale(const ViewState *viewState, const float fitScale) {
	float result;
	switch (viewState->zoomMode) {
		case ViewZoomMode_Fit:
			result = fitScale;
			break;
		case ViewZoomMode_ActualSize:
			result = ViewDefaultScale;
			break;
		case ViewZoomMode_Custom:
			if (viewState->pendingRelativeScale > 0.0f) {
				result = viewState->pendingRelativeScale * fitScale;
			} else {
				result = viewState->customScale > 0.0f ? viewState->customScale : ViewDefaultScale;
			}
			break;
		case ViewZoomMode_ShrinkToFit:
		default:
			result = fitScale < ViewDefaultScale ? fitScale : ViewDefaultScale;
			break;
	}
	return(result);
}

// Left (or top) edge of the displayed picture on one axis: centered when it fits, otherwise the center point at the viewport center, clamped so that no border shows
static float ViewPlaceAxis(const float viewportLength, const float displayedLength, const float center) {
	if (displayedLength <= viewportLength) {
		float centered = ViewCenterOffset(viewportLength, displayedLength);
		return(centered);
	}
	const float half = 0.5f;
	float edge = roundf(viewportLength * half - center * displayedLength);
	float lowestEdge = viewportLength - displayedLength;
	float result = ViewClamp(edge, lowestEdge, 0.0f);
	return(result);
}

extern ViewTransform ComputeViewTransform(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize) {
	float fitScale = ComputeViewFitScale(pictureSize, viewportSize);
	float scale = ViewModeScale(viewState, fitScale);

	float pictureWidth = (float)pictureSize.width;
	float pictureHeight = (float)pictureSize.height;
	float viewportWidth = (float)viewportSize.width;
	float viewportHeight = (float)viewportSize.height;
	float displayedWidth = ViewDisplayedLength(pictureWidth, scale);
	float displayedHeight = ViewDisplayedLength(pictureHeight, scale);

	ViewTransform result;
	result.scale = scale;
	result.fitScale = fitScale;
	result.scaleX = ViewAxisScale(displayedWidth, pictureWidth, scale);
	result.scaleY = ViewAxisScale(displayedHeight, pictureHeight, scale);
	result.imageRect.left = ViewPlaceAxis(viewportWidth, displayedWidth, viewState->centerX);
	result.imageRect.top = ViewPlaceAxis(viewportHeight, displayedHeight, viewState->centerY);
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

// Zoom steps of the + and - keys (plan section 2.6), the fit scale joins them
static const float ViewZoomSteps[] = { 1.0f / 8.0f, 1.0f / 4.0f, 1.0f / 3.0f, 1.0f / 2.0f, 2.0f / 3.0f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f, 12.0f, 16.0f, 24.0f, 32.0f };
// A step this close (relative) to the current scale counts as the current one, so float noise never makes it the next step
static const float ViewStepTolerance = 0.001f;

extern void ComputeViewScaleLimits(const float fitScale, float *outMinimum, float *outMaximum) {
	const float minimumFitShare = 0.5f;
	const float largestScale = 32.0f;
	float fitMinimum = fitScale * minimumFitShare;
	*outMinimum = fitMinimum < ViewDefaultScale ? fitMinimum : ViewDefaultScale;
	*outMaximum = fitScale > largestScale ? fitScale : largestScale;
}

// Fit at the fit scale, ActualSize at 1:1, Custom for every other scale
static ViewZoomMode ViewModeForScale(const float scale, const float fitScale) {
	if (scale == fitScale) {
		return(ViewZoomMode_Fit);
	}
	if (scale == ViewDefaultScale) {
		return(ViewZoomMode_ActualSize);
	}
	return(ViewZoomMode_Custom);
}

extern float ComputeViewStepScale(const float currentScale, const float fitScale, const int direction, ViewZoomMode *outMode) {
	float minimum;
	float maximum;
	ComputeViewScaleLimits(fitScale, &minimum, &maximum);
	uint32_t zoomStepCount = (uint32_t)(sizeof(ViewZoomSteps) / sizeof(ViewZoomSteps[0]));
	float upperBound = currentScale * (1.0f + ViewStepTolerance);
	float lowerBound = currentScale * (1.0f - ViewStepTolerance);
	float result = currentScale;
	bool isFound = false;
	// Every zoom step and, as the last candidate, the fit scale
	for (uint32_t candidateIndex = 0; candidateIndex <= zoomStepCount; ++candidateIndex) {
		float candidate = candidateIndex < zoomStepCount ? ViewZoomSteps[candidateIndex] : fitScale;
		if (candidate < minimum || candidate > maximum) {
			continue;
		}
		bool isAbove = candidate > upperBound && (!isFound || candidate < result);
		bool isBelow = candidate < lowerBound && (!isFound || candidate > result);
		if ((direction > 0 && isAbove) || (direction < 0 && isBelow)) {
			result = candidate;
			isFound = true;
		}
	}
	if (!isFound) {
		result = ViewClamp(currentScale, minimum, maximum);
	}
	*outMode = ViewModeForScale(result, fitScale);
	return(result);
}

extern float ComputeViewWheelScale(const float currentScale, const float fitScale, const float wheelDelta, ViewZoomMode *outMode) {
	const float wheelZoomFactorPerNotch = 1.2f;
	float minimum;
	float maximum;
	ComputeViewScaleLimits(fitScale, &minimum, &maximum);
	float factor = powf(wheelZoomFactorPerNotch, wheelDelta);
	float target = currentScale * factor;
	// Magnet: of the fit scale and 1:1, the one the turn passes over first. A turn that ends just short of it (one notch down from one notch up) counts as reaching it.
	const float snapScales[] = { fitScale, ViewDefaultScale };
	float result = target;
	float nearestSnapDistance = 0.0f;
	bool isSnapped = false;
	for (uint32_t snapIndex = 0; snapIndex < (uint32_t)(sizeof(snapScales) / sizeof(snapScales[0])); ++snapIndex) {
		float snap = snapScales[snapIndex];
		float snapUpper = snap * (1.0f + ViewStepTolerance);
		float snapLower = snap * (1.0f - ViewStepTolerance);
		bool isPassedUpwards = currentScale < snapLower && target > snapLower;
		bool isPassedDownwards = currentScale > snapUpper && target < snapUpper;
		float snapDistance = fabsf(snap - currentScale);
		if ((isPassedUpwards || isPassedDownwards) && (!isSnapped || snapDistance < nearestSnapDistance)) {
			result = snap;
			nearestSnapDistance = snapDistance;
			isSnapped = true;
		}
	}
	result = ViewClamp(result, minimum, maximum);
	*outMode = ViewModeForScale(result, fitScale);
	return(result);
}

// Normalized center that puts the displayed picture edge at edge, clamped to the range where no border shows, 0.5 when the picture fits on this axis
static float ViewAxisCenter(const float viewportLength, const float displayedLength, const float edge) {
	const float half = 0.5f;
	if (displayedLength <= viewportLength || displayedLength <= 0.0f) {
		return(half);
	}
	float center = (viewportLength * half - edge) / displayedLength;
	float lowestCenter = viewportLength * half / displayedLength;
	float highestCenter = 1.0f - lowestCenter;
	float result = ViewClamp(center, lowestCenter, highestCenter);
	return(result);
}

static bool ViewIsSizeKnown(const ViewSize pictureSize, const ViewSize viewportSize) {
	bool result = pictureSize.width > 0 && pictureSize.height > 0 && viewportSize.width > 0 && viewportSize.height > 0;
	return(result);
}

extern ViewState ComputeViewZoomAtPoint(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize, const ViewZoomMode newMode, const float newScale, const float pointX, const float pointY) {
	ViewTransform current = ComputeViewTransform(viewState, pictureSize, viewportSize);
	ViewState result = ViewMakeState(newMode, newScale, viewState->centerX, viewState->centerY);
	if (!ViewIsSizeKnown(pictureSize, viewportSize)) {
		return(result);
	}
	float scale = ViewModeScale(&result, current.fitScale);
	result.customScale = scale;
	float displayedWidth = ViewDisplayedLength((float)pictureSize.width, scale);
	float displayedHeight = ViewDisplayedLength((float)pictureSize.height, scale);
	// The fraction of the picture under the point stays under the point
	float fractionX = (pointX - current.imageRect.left) / current.imageRect.width;
	float fractionY = (pointY - current.imageRect.top) / current.imageRect.height;
	float newLeft = pointX - fractionX * displayedWidth;
	float newTop = pointY - fractionY * displayedHeight;
	result.centerX = ViewAxisCenter((float)viewportSize.width, displayedWidth, newLeft);
	result.centerY = ViewAxisCenter((float)viewportSize.height, displayedHeight, newTop);
	return(result);
}

extern ViewState ComputeViewPan(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize, const float deltaX, const float deltaY) {
	ViewState result = *viewState;
	if (!ViewIsSizeKnown(pictureSize, viewportSize)) {
		return(result);
	}
	// From the center as shown, so a pan against a clamped edge moves the picture at once when it turns around
	ViewTransform current = ComputeViewTransform(viewState, pictureSize, viewportSize);
	float movedLeft = current.imageRect.left + deltaX;
	float movedTop = current.imageRect.top + deltaY;
	result.centerX = ViewAxisCenter((float)viewportSize.width, current.imageRect.width, movedLeft);
	result.centerY = ViewAxisCenter((float)viewportSize.height, current.imageRect.height, movedTop);
	return(result);
}

extern ViewState ComputeViewResolved(const ViewState *viewState, const ViewSize pictureSize, const ViewSize viewportSize) {
	ViewState result = *viewState;
	bool isPending = viewState->zoomMode == ViewZoomMode_Custom && viewState->pendingRelativeScale > 0.0f;
	if (isPending && ViewIsSizeKnown(pictureSize, viewportSize)) {
		float fitScale = ComputeViewFitScale(pictureSize, viewportSize);
		result.customScale = viewState->pendingRelativeScale * fitScale;
		result.pendingRelativeScale = 0.0f;
	}
	return(result);
}

extern ViewState ComputeViewForNextPicture(const ViewState *viewState, const ViewPersistence persistence, const ViewState *startView, const ViewSize pictureSize, const ViewSize viewportSize) {
	if (persistence == ViewPersistence_Reset) {
		return(*startView);
	}
	if (!ViewIsSizeKnown(pictureSize, viewportSize)) {
		return(*viewState);
	}
	// The center as shown (an axis that fits is centered) and the scale as shown, relative to the fit scale or as it is
	ViewTransform current = ComputeViewTransform(viewState, pictureSize, viewportSize);
	ViewState result = *viewState;
	result.centerX = ViewAxisCenter((float)viewportSize.width, current.imageRect.width, current.imageRect.left);
	result.centerY = ViewAxisCenter((float)viewportSize.height, current.imageRect.height, current.imageRect.top);
	if (viewState->zoomMode == ViewZoomMode_Custom) {
		if (persistence == ViewPersistence_KeepRelative) {
			result.pendingRelativeScale = current.scale / current.fitScale;
		} else {
			result.customScale = current.scale;
			result.pendingRelativeScale = 0.0f;
		}
	}
	return(result);
}

extern uint32_t ComputeViewSourceLevel(const float scale, const uint32_t levelCount, const uint32_t firstLevel) {
	// The source level must be at least eight times as large as the displayed picture: from twice on the 2:1 kernels of the levels
	// dampen the finest details that are still shown, which differs visibly from level 0 (iteration 4, plan section 2.3)
	const float largestLevelScale = 0.125f;
	const float levelScaleFactor = 2.0f;
	uint32_t lastLevel = levelCount > 0 ? levelCount - 1 : 0;
	uint32_t lowestLevel = firstLevel < lastLevel ? firstLevel : lastLevel;
	if (!(scale > 0.0f)) {
		return(lowestLevel);
	}
	uint32_t level = 0;
	float levelScale = scale;
	while (level < lastLevel) {
		float nextLevelScale = levelScale * levelScaleFactor;
		if (nextLevelScale > largestLevelScale) {
			break;
		}
		levelScale = nextLevelScale;
		++level;
	}
	uint32_t result = level > lowestLevel ? level : lowestLevel;
	return(result);
}

typedef struct ViewAxisPlacement {
	float offset;
	float firstCoverage;
	float lastCoverage;
} ViewAxisPlacement;

// One displayed axis: the stored axis it shows and whether it runs backwards come from the steps of the orientation mapping
static ViewAxisPlacement ViewComputeAxisPlacement(const int32_t stepX, const int32_t stepY, const ViewSize storedSize, const ViewSize storedLevelSize, const float levelFactor) {
	bool showsStoredX = stepX != 0;
	bool isMirrored = stepX < 0 || stepY < 0;
	float pictureLength = showsStoredX ? (float)storedSize.width : (float)storedSize.height;
	float levelLength = showsStoredX ? (float)storedLevelSize.width : (float)storedLevelSize.height;
	float overhang = levelLength - pictureLength / levelFactor;
	float edgeCoverage = 1.0f - overhang;
	ViewAxisPlacement result;
	result.offset = isMirrored ? overhang : 0.0f;
	result.firstCoverage = isMirrored ? edgeCoverage : 1.0f;
	result.lastCoverage = isMirrored ? 1.0f : edgeCoverage;
	return(result);
}

extern ViewLevelPlacement ComputeViewLevelPlacement(const uint32_t orientation, const ViewSize storedSize, const ViewSize storedLevelSize, const uint32_t level) {
	float levelFactor = ldexpf(1.0f, (int)level);
	ViewOrientationMapping mapping = ComputeViewOrientationMapping(orientation, storedLevelSize);
	ViewAxisPlacement columns = ViewComputeAxisPlacement(mapping.stepUX, mapping.stepUY, storedSize, storedLevelSize, levelFactor);
	ViewAxisPlacement rows = ViewComputeAxisPlacement(mapping.stepVX, mapping.stepVY, storedSize, storedLevelSize, levelFactor);
	ViewLevelPlacement result;
	result.offsetU = columns.offset;
	result.offsetV = rows.offset;
	result.firstCoverageU = columns.firstCoverage;
	result.lastCoverageU = columns.lastCoverage;
	result.firstCoverageV = rows.firstCoverage;
	result.lastCoverageV = rows.lastCoverage;
	return(result);
}

#endif // VIEW_TRANSFORM_IMPLEMENTATION
