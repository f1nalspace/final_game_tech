/*
Name:
	FPL_ImageViewer | Self test

Description:
	Checks for --selftest that need neither a window nor OpenGL, so they also run on a machine without a GPU or under an emulator.
	Prints every failed check and a summary, RunSelfTest() returns the process exit code (0 = all passed, 1 = at least one failed).
	Include once, in the translation unit that implements final_platform_layer.h and viewtransform.h.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef SELFTEST_H
#define SELFTEST_H

#include <math.h>

#include <final_platform_layer.h>

#include "viewtransform.h"

typedef struct SelfTest {
	const char* groupName;
	uint32_t checkCount;
	uint32_t failedCount;
} SelfTest;

// Largest difference two float results of the view math may have and still count as equal
static const float SelfTestFloatTolerance = 0.0001f;

static void SelfTestCheck(SelfTest* test, const bool condition, const char* description) {
	++test->checkCount;
	if (!condition) {
		++test->failedCount;
		fplConsoleFormatError("[%s] FAILED: %s\n", test->groupName, description);
	}
}

static void SelfTestCheckNear(SelfTest* test, const float actual, const float expected, const char* description) {
	float difference = fabsf(actual - expected);
	bool isNear = difference <= SelfTestFloatTolerance;
	++test->checkCount;
	if (!isNear) {
		++test->failedCount;
		fplConsoleFormatError("[%s] FAILED: %s (expected %f, got %f)\n", test->groupName, description, expected, actual);
	}
}

static void SelfTestCheckRect(SelfTest* test, const ViewRect actual, const float left, const float top, const float width, const float height, const char* description) {
	char text[256];
	fplStringFormat(text, fplArrayCount(text), "%s left", description);
	SelfTestCheckNear(test, actual.left, left, text);
	fplStringFormat(text, fplArrayCount(text), "%s top", description);
	SelfTestCheckNear(test, actual.top, top, text);
	fplStringFormat(text, fplArrayCount(text), "%s width", description);
	SelfTestCheckNear(test, actual.width, width, text);
	fplStringFormat(text, fplArrayCount(text), "%s height", description);
	SelfTestCheckNear(test, actual.height, height, text);
}

static ViewTransform SelfTestComputeTransform(const ViewZoomMode zoomMode, const float customScale, const uint32_t pictureWidth, const uint32_t pictureHeight, const uint32_t viewportWidth, const uint32_t viewportHeight) {
	ViewState viewState = fplStructInit(ViewState, zoomMode, customScale);
	ViewSize pictureSize = fplStructInit(ViewSize, pictureWidth, pictureHeight);
	ViewSize viewportSize = fplStructInit(ViewSize, viewportWidth, viewportHeight);
	ViewTransform result = ComputeViewTransform(&viewState, pictureSize, viewportSize);
	return(result);
}

static void SelfTestViewTransform(SelfTest* test) {
	test->groupName = "ViewTransform";

	// 1:1, centered, the scaling test runner crops (window - picture) / 2 rounded down
	ViewTransform actualSize = SelfTestComputeTransform(ViewZoomMode_ActualSize, 0.0f, 512, 512, 1280, 720);
	SelfTestCheckNear(test, actualSize.scale, 1.0f, "ActualSize scale");
	SelfTestCheckRect(test, actualSize.imageRect, 384.0f, 104.0f, 512.0f, 512.0f, "ActualSize 512 in 1280x720 image rect");
	SelfTestCheckRect(test, actualSize.visibleSourceRect, 0.0f, 0.0f, 512.0f, 512.0f, "ActualSize 512 in 1280x720 visible source");

	ViewTransform oddDifference = SelfTestComputeTransform(ViewZoomMode_ActualSize, 0.0f, 511, 511, 1280, 720);
	SelfTestCheckRect(test, oddDifference.imageRect, 384.0f, 104.0f, 511.0f, 511.0f, "ActualSize 511 in 1280x720 rounds the origin down");

	// Larger than the viewport at 1:1: centered, only the middle is visible
	ViewTransform actualSizeLarge = SelfTestComputeTransform(ViewZoomMode_ActualSize, 0.0f, 2000, 1000, 1000, 500);
	SelfTestCheckRect(test, actualSizeLarge.imageRect, -500.0f, -250.0f, 2000.0f, 1000.0f, "ActualSize 2000x1000 in 1000x500 image rect");
	SelfTestCheckRect(test, actualSizeLarge.visibleSourceRect, 500.0f, 250.0f, 1000.0f, 500.0f, "ActualSize 2000x1000 in 1000x500 visible source");

	// Shrink to fit: small pictures stay 1:1, large ones are fitted
	ViewTransform shrinkSmall = SelfTestComputeTransform(ViewZoomMode_ShrinkToFit, 0.0f, 300, 200, 1280, 720);
	SelfTestCheckNear(test, shrinkSmall.scale, 1.0f, "ShrinkToFit keeps small pictures at 1:1");
	SelfTestCheckRect(test, shrinkSmall.imageRect, 490.0f, 260.0f, 300.0f, 200.0f, "ShrinkToFit 300x200 in 1280x720 image rect");

	ViewTransform shrinkLarge = SelfTestComputeTransform(ViewZoomMode_ShrinkToFit, 0.0f, 4032, 3024, 1280, 720);
	SelfTestCheckNear(test, shrinkLarge.scale, 720.0f / 3024.0f, "ShrinkToFit 4032x3024 in 1280x720 scale");
	SelfTestCheckRect(test, shrinkLarge.imageRect, 160.0f, 0.0f, 960.0f, 720.0f, "ShrinkToFit 4032x3024 in 1280x720 image rect");
	SelfTestCheckRect(test, shrinkLarge.visibleSourceRect, 0.0f, 0.0f, 4032.0f, 3024.0f, "ShrinkToFit 4032x3024 in 1280x720 visible source");

	// Fit also upscales
	ViewTransform fitSmall = SelfTestComputeTransform(ViewZoomMode_Fit, 0.0f, 32, 32, 256, 128);
	SelfTestCheckNear(test, fitSmall.scale, 4.0f, "Fit 32x32 in 256x128 scale");
	SelfTestCheckRect(test, fitSmall.imageRect, 64.0f, 0.0f, 128.0f, 128.0f, "Fit 32x32 in 256x128 image rect");

	// The displayed size is rounded to whole pixels on each axis, like ImageMagick -resize 358x268! for 1023x767 at 35 %
	ViewTransform fitOddAspect = SelfTestComputeTransform(ViewZoomMode_Fit, 0.0f, 1023, 767, 358, 268);
	SelfTestCheckRect(test, fitOddAspect.imageRect, 0.0f, 0.0f, 357.0f, 268.0f, "Fit 1023x767 in 358x268 keeps the aspect ratio on whole pixels");
	SelfTestCheckNear(test, fitOddAspect.scaleX, 357.0f / 1023.0f, "Fit 1023x767 in 358x268 displayed scale x");
	SelfTestCheckNear(test, fitOddAspect.scaleY, 268.0f / 767.0f, "Fit 1023x767 in 358x268 displayed scale y");
	ViewTransform customOddAspect = SelfTestComputeTransform(ViewZoomMode_Custom, 0.35f, 1023, 767, 358, 268);
	SelfTestCheckRect(test, customOddAspect.imageRect, 0.0f, 0.0f, 358.0f, 268.0f, "Custom 0.35 of 1023x767 fills 358x268");
	SelfTestCheckNear(test, customOddAspect.scaleX, 358.0f / 1023.0f, "Custom 0.35 of 1023x767 displayed scale x");
	SelfTestCheckNear(test, customOddAspect.scaleY, 268.0f / 767.0f, "Custom 0.35 of 1023x767 displayed scale y");

	// Custom scale, and an invalid one falls back to 1:1
	ViewTransform custom = SelfTestComputeTransform(ViewZoomMode_Custom, 2.5f, 100, 100, 1000, 1000);
	SelfTestCheckRect(test, custom.imageRect, 375.0f, 375.0f, 250.0f, 250.0f, "Custom 2.5 image rect");
	ViewTransform customTiny = SelfTestComputeTransform(ViewZoomMode_Custom, 0.01f, 30, 30, 100, 100);
	SelfTestCheckRect(test, customTiny.imageRect, 49.0f, 49.0f, 1.0f, 1.0f, "Custom 0.01 of 30x30 keeps one pixel");
	ViewTransform customInvalid = SelfTestComputeTransform(ViewZoomMode_Custom, 0.0f, 100, 100, 1000, 1000);
	SelfTestCheckNear(test, customInvalid.scale, 1.0f, "Custom scale 0 falls back to 1");

	// Degenerated sizes must not produce NaN or infinity
	ViewTransform emptyPicture = SelfTestComputeTransform(ViewZoomMode_Fit, 0.0f, 0, 0, 640, 480);
	SelfTestCheck(test, isfinite(emptyPicture.scale) && isfinite(emptyPicture.visibleSourceRect.width), "Empty picture gives finite values");
	ViewTransform emptyViewport = SelfTestComputeTransform(ViewZoomMode_Fit, 0.0f, 640, 480, 0, 0);
	SelfTestCheck(test, isfinite(emptyViewport.scale) && isfinite(emptyViewport.imageRect.left), "Empty viewport gives finite values");

	// Sweep: whole pixel rectangles in every mode, fitted pictures stay inside the viewport and touch it on one axis
	const ViewZoomMode sweepModes[] = { ViewZoomMode_ShrinkToFit, ViewZoomMode_Fit, ViewZoomMode_ActualSize, ViewZoomMode_Custom };
	const float sweepCustomScale = 0.37f;
	const uint32_t firstSweepSize = 1;
	const uint32_t lastSweepSize = 700;
	const uint32_t pictureSweepStep = 37;
	const uint32_t viewportSweepStep = 53;
	const uint32_t sweepViewportHeightNumerator = 3;
	const uint32_t sweepViewportHeightDenominator = 4;
	uint32_t wholePixelFailures = 0;
	uint32_t insideFailures = 0;
	uint32_t touchFailures = 0;
	for (uint32_t modeIndex = 0; modeIndex < fplArrayCount(sweepModes); ++modeIndex) {
		ViewZoomMode mode = sweepModes[modeIndex];
		for (uint32_t pictureWidth = firstSweepSize; pictureWidth <= lastSweepSize; pictureWidth += pictureSweepStep) {
			uint32_t pictureHeight = lastSweepSize + firstSweepSize - pictureWidth;
			for (uint32_t viewportWidth = firstSweepSize; viewportWidth <= lastSweepSize; viewportWidth += viewportSweepStep) {
				uint32_t viewportHeight = (viewportWidth * sweepViewportHeightNumerator) / sweepViewportHeightDenominator + firstSweepSize;
				ViewTransform transform = SelfTestComputeTransform(mode, sweepCustomScale, pictureWidth, pictureHeight, viewportWidth, viewportHeight);
				const ViewRect* rect = &transform.imageRect;
				bool isWholeOrigin = floorf(rect->left) == rect->left && floorf(rect->top) == rect->top;
				bool isWholeSize = floorf(rect->width) == rect->width && floorf(rect->height) == rect->height;
				if (!isWholeOrigin || !isWholeSize) {
					++wholePixelFailures;
				}
				if (mode == ViewZoomMode_Fit) {
					float right = transform.imageRect.left + transform.imageRect.width;
					float bottom = transform.imageRect.top + transform.imageRect.height;
					bool isInside = transform.imageRect.left >= 0.0f && transform.imageRect.top >= 0.0f && right <= (float)viewportWidth + SelfTestFloatTolerance && bottom <= (float)viewportHeight + SelfTestFloatTolerance;
					if (!isInside) {
						++insideFailures;
					}
					float widthGap = fabsf(transform.imageRect.width - (float)viewportWidth);
					float heightGap = fabsf(transform.imageRect.height - (float)viewportHeight);
					bool touchesViewport = widthGap <= SelfTestFloatTolerance * (float)viewportWidth || heightGap <= SelfTestFloatTolerance * (float)viewportHeight;
					if (!touchesViewport) {
						++touchFailures;
					}
				}
			}
		}
	}
	SelfTestCheck(test, wholePixelFailures == 0, "Sweep: the picture rectangle is always on whole pixels");
	SelfTestCheck(test, insideFailures == 0, "Sweep: a fitted picture stays inside the viewport");
	SelfTestCheck(test, touchFailures == 0, "Sweep: a fitted picture fills the viewport on one axis");
}

static int RunSelfTest() {
	SelfTest test = fplZeroInit;
	SelfTestViewTransform(&test);
	fplConsoleFormatOut("Self test: %u checks, %u failed\n", test.checkCount, test.failedCount);
	int result = test.failedCount == 0 ? 0 : 1;
	return(result);
}

#endif // SELFTEST_H
