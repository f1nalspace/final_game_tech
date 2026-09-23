/*
Name:
	FPL_ImageViewer | Self test

Description:
	Checks for --selftest that need neither a window nor OpenGL, so they also run on a machine without a GPU or under an emulator.
	Prints every failed check and a summary, RunSelfTest() returns the process exit code (0 = all passed, 1 = at least one failed).
	Include once, in the translation unit that implements final_platform_layer.h, viewtransform.h, resamplepipeline.h, imagepyramid.h and the image loaders.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef SELFTEST_H
#define SELFTEST_H

#include <math.h>

#include <final_platform_layer.h>

#include "viewtransform.h"
#include "resamplepipeline.h"
#include "imageloader.h"
#include "imageloader_stb.h"
#include "imageloader_pnm.h"
#include "imageloader_bmp.h"
#include "imagepyramid.h"

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

static void SelfTestCheckRange(SelfTest* test, const ResampleSourceRange actual, const int32_t first, const int32_t end, const char* description) {
	bool isEqual = actual.first == first && actual.end == end;
	++test->checkCount;
	if (!isEqual) {
		++test->failedCount;
		fplConsoleFormatError("[%s] FAILED: %s (expected [%d, %d), got [%d, %d))\n", test->groupName, description, first, end, actual.first, actual.end);
	}
}

static void SelfTestResample(SelfTest* test) {
	test->groupName = "Resample";

	// Every kernel and background is found by its key, ignoring case, and unknown keys are rejected
	uint32_t lookupFailures = 0;
	for (int kernelIndex = 0; kernelIndex < ResampleKernel_Count; ++kernelIndex) {
		const ResampleKernelDefinition* definition = ResampleGetKernelDefinition((ResampleKernel)kernelIndex);
		ResampleKernel found = ResampleKernel_Count;
		if (!ResampleFindKernel(definition->key, &found) || found != (ResampleKernel)kernelIndex) {
			++lookupFailures;
		}
	}
	for (int backgroundIndex = 0; backgroundIndex < ResampleBackground_Count; ++backgroundIndex) {
		const ResampleBackgroundDefinition* definition = ResampleGetBackgroundDefinition((ResampleBackground)backgroundIndex);
		ResampleBackground found = ResampleBackground_Count;
		if (!ResampleFindBackground(definition->key, &found) || found != (ResampleBackground)backgroundIndex) {
			++lookupFailures;
		}
	}
	SelfTestCheck(test, lookupFailures == 0, "Every kernel and background is found by its key");
	ResampleKernel upperCaseKernel = ResampleKernel_Count;
	bool isUpperCaseFound = ResampleFindKernel("MITCHELL", &upperCaseKernel);
	SelfTestCheck(test, isUpperCaseFound && upperCaseKernel == ResampleKernel_Mitchell, "Kernel keys ignore case");
	ResampleKernel unknownKernel = ResampleKernel_Count;
	SelfTestCheck(test, !ResampleFindKernel("mitch", &unknownKernel), "A partial kernel key is rejected");

	// Source rows a vertical pass reads, including one row of margin on both sides:
	// Mitchell at 0.5 widens the support to 4, the first output center 0.5 maps to 1, the last one 99.5 to 199
	ResampleSourceRange wholePicture = ResampleComputeSourceRange(ResampleKernel_Mitchell, 0.5f, 0.0f, 0, 100, 200);
	SelfTestCheckRange(test, wholePicture, 0, 200, "Mitchell 0.5, all 100 output rows of 200 source rows");
	// Output rows 10..19 map to 21..39, taps floor(21 - 4 + 0.5) = 17 up to floor(39 + 4 + 0.5) = 43
	ResampleSourceRange someRows = ResampleComputeSourceRange(ResampleKernel_Mitchell, 0.5f, 0.0f, 10, 20, 1000);
	SelfTestCheckRange(test, someRows, 16, 44, "Mitchell 0.5, output rows 10..19");
	// Catmull-Rom at 4 is not widened, the picture starts 100 output pixels before the output area: 0.5 maps to 25.125, 7.5 to 26.875
	ResampleSourceRange upscaled = ResampleComputeSourceRange(ResampleKernel_CatmullRom, 4.0f, -100.0f, 0, 8, 1000);
	SelfTestCheckRange(test, upscaled, 22, 30, "Catmull-Rom 4, picture 100 pixels before the output");
	// Nearest reads one tap per output pixel: 0.5 maps to 2, 3.5 to 14
	ResampleSourceRange nearest = ResampleComputeSourceRange(ResampleKernel_Nearest, 0.25f, 0.0f, 0, 4, 16);
	SelfTestCheckRange(test, nearest, 1, 16, "Nearest 0.25");
	ResampleSourceRange empty = ResampleComputeSourceRange(ResampleKernel_Mitchell, 0.5f, 0.0f, 5, 5, 100);
	SelfTestCheckRange(test, empty, 0, 0, "No output rows read nothing");
}

static void SelfTestOrientation(SelfTest* test) {
	test->groupName = "Orientation";
	const uint32_t storedWidth = 3;
	const uint32_t storedHeight = 2;
	const uint32_t firstOrientation = 1;
	const uint32_t lastOrientation = 8;
	const uint32_t firstTurnedOrientation = 5;
	ViewSize storedSize = fplStructInit(ViewSize, storedWidth, storedHeight);
	// Stored pixel that the displayed top-left pixel shows, per EXIF orientation 1..8 (row 0 / column 0 as the specification places them)
	const int32_t expectedTopLeftX[] = { 0, 0, 2, 2, 0, 0, 0, 2, 2 };
	const int32_t expectedTopLeftY[] = { 0, 0, 0, 1, 1, 0, 1, 1, 0 };
	uint32_t failures = 0;
	for (uint32_t orientation = firstOrientation; orientation <= lastOrientation; ++orientation) {
		ViewSize displayedSize = ComputeViewOrientedSize(orientation, storedSize);
		bool isTurned = orientation >= firstTurnedOrientation;
		uint32_t expectedWidth = isTurned ? storedHeight : storedWidth;
		if (displayedSize.width != expectedWidth || displayedSize.width * displayedSize.height != storedWidth * storedHeight) {
			++failures;
			continue;
		}
		ViewOrientationMapping mapping = ComputeViewOrientationMapping(orientation, storedSize);
		if (mapping.originX != expectedTopLeftX[orientation] || mapping.originY != expectedTopLeftY[orientation]) {
			++failures;
		}
		// Every displayed pixel lands on a different stored pixel inside the picture
		bool isHit[3 * 2] = { false };
		for (uint32_t v = 0; v < displayedSize.height; ++v) {
			for (uint32_t u = 0; u < displayedSize.width; ++u) {
				int32_t x = mapping.originX + (int32_t)u * mapping.stepUX + (int32_t)v * mapping.stepVX;
				int32_t y = mapping.originY + (int32_t)u * mapping.stepUY + (int32_t)v * mapping.stepVY;
				bool isInside = x >= 0 && y >= 0 && x < (int32_t)storedWidth && y < (int32_t)storedHeight;
				if (!isInside || isHit[y * (int32_t)storedWidth + x]) {
					++failures;
				} else {
					isHit[y * (int32_t)storedWidth + x] = true;
				}
			}
		}
	}
	SelfTestCheck(test, failures == 0, "Orientations 1..8: sizes, corners and a one to one mapping");
}

static void SelfTestLoaderIds(SelfTest* test) {
	test->groupName = "Loader ids";
	const ImageLoader* loaders[] = { ImageLoaderStbGet(), ImageLoaderPnmGet(), ImageLoaderBmpGet() };
	const char* idTexts[] = { IMAGE_LOADER_STB_ID_TEXT, IMAGE_LOADER_PNM_ID_TEXT, IMAGE_LOADER_BMP_ID_TEXT };
	for (size_t loaderIndex = 0; loaderIndex < fplArrayCount(loaders); ++loaderIndex) {
		ImageLoaderId parsed;
		bool isParsed = ImageLoaderIdParse(idTexts[loaderIndex], &parsed);
		SelfTestCheck(test, isParsed && ImageLoaderIdIsEqual(&parsed, &loaders[loaderIndex]->id), "The documented id text matches the id bytes of the loader");
		char formatted[IMAGE_LOADER_ID_TEXT_SIZE];
		ImageLoaderIdFormat(&loaders[loaderIndex]->id, formatted, sizeof(formatted));
		SelfTestCheck(test, strcmp(formatted, idTexts[loaderIndex]) == 0, "Formatting the id gives the documented text");
	}
	ImageLoaderId braced;
	bool isBracedParsed = ImageLoaderIdParse("{854C10CA-79DC-4202-B064-586AF81A96E0}", &braced);
	SelfTestCheck(test, isBracedParsed && ImageLoaderIdIsEqual(&braced, &loaders[0]->id), "Braces and upper case are accepted");
	const char* invalidIds[] = {
		"",
		"854c10ca79dc4202b064586af81a96e0",
		"854c10ca-79dc-4202-b064-586af81a96e",
		"854c10ca-79dc-4202-b064-586af81a96e0x",
		"{854c10ca-79dc-4202-b064-586af81a96e0",
		"854c10ca-79dc-4202-b064-586af81a96eg",
		"854c10ca-79dc-4202-b064586a-f81a96e0",
	};
	uint32_t acceptedInvalid = 0;
	for (size_t index = 0; index < fplArrayCount(invalidIds); ++index) {
		ImageLoaderId id;
		if (ImageLoaderIdParse(invalidIds[index], &id)) {
			++acceptedInvalid;
			fplConsoleFormatError("[%s] accepted invalid id '%s'\n", test->groupName, invalidIds[index]);
		}
	}
	SelfTestCheck(test, acceptedInvalid == 0, "Invalid id texts are rejected");
}

static void SelfTestCheckCandidates(SelfTest* test, const ImageLoaderRegistry* registry, const uint8_t* header, const size_t headerSize, const char* extension, const int32_t* expected, const uint32_t expectedCount, const char* description) {
	int32_t candidates[IMAGE_LOADER_MAX_COUNT];
	uint32_t count = ImageLoaderRegistrySelect(registry, header, headerSize, extension, candidates, IMAGE_LOADER_MAX_COUNT);
	bool isEqual = count == expectedCount;
	for (uint32_t index = 0; isEqual && index < count; ++index) {
		isEqual = candidates[index] == expected[index];
	}
	SelfTestCheck(test, isEqual, description);
}

static void SelfTestLoaderSelection(SelfTest* test) {
	test->groupName = "Loader selection";
	const int32_t stbEntry = 0;
	const int32_t pnmEntry = 1;
	const int32_t bmpEntry = 2;
	ImageLoaderRegistry registry;
	ImageLoaderRegistryInit(&registry);
	char message[IMAGE_LOADER_MESSAGE_SIZE];
	bool isRegistered = ImageLoaderRegistryAdd(&registry, ImageLoaderStbGet(), message, sizeof(message)) && ImageLoaderRegistryAdd(&registry, ImageLoaderPnmGet(), message, sizeof(message)) && ImageLoaderRegistryAdd(&registry, ImageLoaderBmpGet(), message, sizeof(message));
	SelfTestCheck(test, isRegistered, "The three loaders register");
	SelfTestCheck(test, !ImageLoaderRegistryAdd(&registry, ImageLoaderStbGet(), message, sizeof(message)), "A second loader with the same id is rejected");

	const uint8_t pngHeader[] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
	const uint8_t bmpHeader[] = { 'B', 'M', 0, 0 };
	const uint8_t pixmapHeader[] = { 'P', '6', '\n', '1' };
	const uint8_t unknownHeader[] = { 'x', 'y', 'z', 0 };
	const int32_t onlyStb[] = { stbEntry };
	const int32_t stbThenBmp[] = { stbEntry, bmpEntry };
	const int32_t bmpThenStb[] = { bmpEntry, stbEntry };
	const int32_t pnmThenStb[] = { pnmEntry, stbEntry };
	SelfTestCheckCandidates(test, &registry, pngHeader, sizeof(pngHeader), ".jpg", onlyStb, fplArrayCount(onlyStb), "A PNG named .jpg goes to stb_image by its signature");
	SelfTestCheckCandidates(test, &registry, bmpHeader, sizeof(bmpHeader), ".bmp", stbThenBmp, fplArrayCount(stbThenBmp), "A BMP goes to stb_image first, the reference loader follows by order");
	SelfTestCheckCandidates(test, &registry, pixmapHeader, sizeof(pixmapHeader), ".png", pnmThenStb, fplArrayCount(pnmThenStb), "The signature beats the extension (PPM named .png)");
	SelfTestCheckCandidates(test, &registry, unknownHeader, sizeof(unknownHeader), ".png", onlyStb, fplArrayCount(onlyStb), "Unknown content falls back to the extension");
	SelfTestCheckCandidates(test, &registry, unknownHeader, sizeof(unknownHeader), ".xyz", fpl_null, 0, "Unknown content with an unknown extension has no candidate");

	ImageLoaderRegistryPinExtension(&registry, "BMP", bmpEntry);
	SelfTestCheckCandidates(test, &registry, bmpHeader, sizeof(bmpHeader), ".bmp", bmpThenStb, fplArrayCount(bmpThenStb), "A loader pinned for .bmp comes first");
	ImageLoaderRegistryPinGlobal(&registry, pnmEntry);
	SelfTestCheckCandidates(test, &registry, pngHeader, sizeof(pngHeader), ".png", onlyStb, fplArrayCount(onlyStb), "A pinned loader that does not recognize the file is left out");
	registry.isFallbackEnabled = false;
	SelfTestCheckCandidates(test, &registry, bmpHeader, sizeof(bmpHeader), ".bmp", bmpThenStb, 1, "Without fallback only the first candidate remains");

	// Name lookup: ids and unique names, an ambiguous name names the ids
	int32_t foundByName = ImageLoaderRegistryFind(&registry, "PNM", message, sizeof(message));
	int32_t foundById = ImageLoaderRegistryFind(&registry, IMAGE_LOADER_BMP_ID_TEXT, message, sizeof(message));
	SelfTestCheck(test, foundByName == pnmEntry && foundById == bmpEntry, "Loaders are found by name (ignoring case) and by id");
	ImageLoader namesake = *ImageLoaderPnmGet();
	ImageLoaderIdParse("00000000-0000-4000-8000-000000000001", &namesake.id);
	ImageLoaderRegistryAdd(&registry, &namesake, message, sizeof(message));
	int32_t ambiguous = ImageLoaderRegistryFind(&registry, "pnm", message, sizeof(message));
	SelfTestCheck(test, ambiguous < 0 && strstr(message, IMAGE_LOADER_PNM_ID_TEXT) != fpl_null, "An ambiguous name is rejected and the ids are listed");
	ImageLoader wrongVersion = *ImageLoaderPnmGet();
	wrongVersion.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION + 1;
	ImageLoaderIdParse("00000000-0000-4000-8000-000000000002", &wrongVersion.id);
	SelfTestCheck(test, !ImageLoaderRegistryAdd(&registry, &wrongVersion, message, sizeof(message)), "An unknown interface version is rejected");
	ImageLoaderRegistryRelease(&registry);
}

static void SelfTestExif(SelfTest* test) {
	test->groupName = "EXIF";
	// The TIFF blocks of the test images (generate_testimages.sh), orientation entry alone in IFD0
	const uint8_t bigEndianSix[] = { 'M', 'M', 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x01, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const uint8_t littleEndianThree[] = { 'I', 'I', 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	// exif_broken.jpg: 256 entries claimed, the orientation claims 65536 values far outside
	const uint8_t tooManyEntries[] = { 'M', 'M', 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x01, 0x12, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0xff };
	// exif_broken_ifd_offset.jpg: IFD0 far behind the block, a valid looking entry right behind the header must be ignored
	const uint8_t farIfd[] = { 'I', 'I', 0x2a, 0x00, 0xf0, 0xff, 0xff, 0x7f, 0x01, 0x00, 0x12, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const size_t truncatedSize = 5;
	SelfTestCheck(test, ImageExifReadOrientation(bigEndianSix, sizeof(bigEndianSix)) == ImageOrientation_Rotate90, "Big endian orientation 6");
	SelfTestCheck(test, ImageExifReadOrientation(littleEndianThree, sizeof(littleEndianThree)) == ImageOrientation_Rotate180, "Little endian orientation 3");
	SelfTestCheck(test, ImageExifReadOrientation(tooManyEntries, sizeof(tooManyEntries)) == ImageOrientation_Normal, "Broken entry count and value count give orientation 1");
	SelfTestCheck(test, ImageExifReadOrientation(farIfd, sizeof(farIfd)) == ImageOrientation_Normal, "IFD0 outside the block gives orientation 1");
	SelfTestCheck(test, ImageExifReadOrientation(bigEndianSix, truncatedSize) == ImageOrientation_Normal, "A truncated block gives orientation 1");

	// A JPEG header: SOI, APP0 (JFIF), padding bytes, APP1 with the block above, SOS
	uint8_t jpeg[128];
	size_t size = 0;
	const uint8_t start[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x06, 'J', 'F', 'I', 'F', 0xFF, 0xFF, 0xFF, 0xE1 };
	memcpy(jpeg + size, start, sizeof(start));
	size += sizeof(start);
	const size_t lengthFieldSize = 2;
	const size_t exifSignatureSize = 6;
	size_t segmentLength = lengthFieldSize + exifSignatureSize + sizeof(bigEndianSix);
	jpeg[size++] = (uint8_t)(segmentLength >> 8);
	jpeg[size++] = (uint8_t)(segmentLength & 0xFF);
	memcpy(jpeg + size, "Exif\0\0", exifSignatureSize);
	size += exifSignatureSize;
	memcpy(jpeg + size, bigEndianSix, sizeof(bigEndianSix));
	size += sizeof(bigEndianSix);
	const uint8_t scan[] = { 0xFF, 0xDA, 0x00, 0x02 };
	memcpy(jpeg + size, scan, sizeof(scan));
	size += sizeof(scan);
	ImageMemorySource memorySource;
	ImageMemorySourceInit(&memorySource, jpeg, size);
	SelfTestCheck(test, ImageJpegReadOrientation(&memorySource.source) == ImageOrientation_Rotate90, "The JPEG segment walk finds the EXIF block behind APP0 and padding");
	ImageMemorySourceInit(&memorySource, jpeg, size / 2);
	SelfTestCheck(test, ImageJpegReadOrientation(&memorySource.source) == ImageOrientation_Normal, "A JPEG cut inside the EXIF segment gives orientation 1");
}

static bool SelfTestDecode(const ImageLoader* loader, const void* data, const size_t dataSize, ImagePixels* outPixels, ImageLoadResult* outResult) {
	ImageLoaderRegistry registry;
	ImageLoaderRegistryInit(&registry);
	char message[IMAGE_LOADER_MESSAGE_SIZE];
	ImageLoaderRegistryAdd(&registry, loader, message, sizeof(message));
	ImageMemorySource memorySource;
	ImageMemorySourceInit(&memorySource, data, dataSize);
	*outResult = ImageLoaderRegistryDecode(&registry, 0, &memorySource.source, outPixels, message, sizeof(message));
	ImageLoaderRegistryRelease(&registry);
	return(*outResult == ImageLoadResult_Success);
}

static bool SelfTestPixelIs(const ImagePixels* pixels, const uint32_t x, const uint32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	const uint8_t* p = (const uint8_t*)pixels->pixels + (size_t)y * pixels->stride + (size_t)x * 4;
	bool result = p[0] == r && p[1] == g && p[2] == b && p[3] == a;
	return(result);
}

static void SelfTestLoaders(SelfTest* test) {
	test->groupName = "Loaders";
	const ImageLoader* pnm = ImageLoaderPnmGet();
	const ImageLoader* bmp = ImageLoaderBmpGet();
	ImagePixels pixels;
	ImageLoadResult result;

	const uint8_t pixmap[] = { 'P', '6', '\n', '2', ' ', '1', '\n', '2', '5', '5', '\n', 255, 0, 0, 0, 0, 255 };
	bool isPixmap = SelfTestDecode(pnm, pixmap, sizeof(pixmap), &pixels, &result);
	SelfTestCheck(test, isPixmap && SelfTestPixelIs(&pixels, 0, 0, 255, 0, 0, 255) && SelfTestPixelIs(&pixels, 1, 0, 0, 0, 255, 255), "PPM 8 bit");
	free(pixels.pixels);
	// 32768 of 65535 scales to 128
	const uint8_t wideGray[] = { 'P', '5', ' ', '1', ' ', '1', ' ', '6', '5', '5', '3', '5', '\n', 0x80, 0x00 };
	bool isWideGray = SelfTestDecode(pnm, wideGray, sizeof(wideGray), &pixels, &result);
	SelfTestCheck(test, isWideGray && SelfTestPixelIs(&pixels, 0, 0, 128, 128, 128, 255), "PGM 16 bit, big endian samples");
	free(pixels.pixels);
	const char grayAlphaHeader[] = "P7\nWIDTH 1\nHEIGHT 1\nDEPTH 2\nMAXVAL 255\n# comment\nTUPLTYPE GRAYSCALE_ALPHA\nENDHDR\n";
	uint8_t grayAlpha[sizeof(grayAlphaHeader) + 1];
	memcpy(grayAlpha, grayAlphaHeader, sizeof(grayAlphaHeader) - 1);
	grayAlpha[sizeof(grayAlphaHeader) - 1] = 10;
	grayAlpha[sizeof(grayAlphaHeader)] = 20;
	bool isGrayAlpha = SelfTestDecode(pnm, grayAlpha, sizeof(grayAlpha), &pixels, &result);
	SelfTestCheck(test, isGrayAlpha && SelfTestPixelIs(&pixels, 0, 0, 10, 10, 10, 20), "PAM gray with alpha and a comment");
	free(pixels.pixels);
	SelfTestDecode(pnm, pixmap, sizeof(pixmap) - 1, &pixels, &result);
	SelfTestCheck(test, result == ImageLoadResult_Corrupt, "A truncated PPM is corrupt");

	// BMP 2x2, 24 bit bottom-up: the first row in the file is the bottom row, rows are padded to 4 bytes
	uint8_t bmp24[54 + 16] = { 0 };
	const uint8_t bmp24Header[] = { 'B', 'M', 70, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 1, 0, 24, 0 };
	memcpy(bmp24, bmp24Header, sizeof(bmp24Header));
	const uint8_t bmp24Rows[] = { 1, 2, 3, 4, 5, 6, 0, 0, 7, 8, 9, 10, 11, 12, 0, 0 };
	memcpy(bmp24 + 54, bmp24Rows, sizeof(bmp24Rows));
	bool isBmp24 = SelfTestDecode(bmp, bmp24, sizeof(bmp24), &pixels, &result);
	SelfTestCheck(test, isBmp24 && SelfTestPixelIs(&pixels, 0, 0, 9, 8, 7, 255) && SelfTestPixelIs(&pixels, 1, 1, 6, 5, 4, 255), "BMP 24 bit bottom-up, BGR and row padding");
	free(pixels.pixels);
	// 1x2, 32 bit top-down with only zero alpha bytes: opaque like stb_image
	uint8_t bmp32[54 + 8] = { 0 };
	const uint8_t bmp32Header[] = { 'B', 'M', 62, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0, 1, 0, 0, 0, 0xFE, 0xFF, 0xFF, 0xFF, 1, 0, 32, 0 };
	memcpy(bmp32, bmp32Header, sizeof(bmp32Header));
	const uint8_t bmp32Rows[] = { 1, 2, 3, 0, 4, 5, 6, 0 };
	memcpy(bmp32 + 54, bmp32Rows, sizeof(bmp32Rows));
	bool isBmp32 = SelfTestDecode(bmp, bmp32, sizeof(bmp32), &pixels, &result);
	SelfTestCheck(test, isBmp32 && SelfTestPixelIs(&pixels, 0, 0, 3, 2, 1, 255) && SelfTestPixelIs(&pixels, 0, 1, 6, 5, 4, 255), "BMP 32 bit top-down, zero alpha becomes opaque");
	free(pixels.pixels);
	uint8_t bmp8[sizeof(bmp24)];
	memcpy(bmp8, bmp24, sizeof(bmp24));
	const size_t bitCountOffset = 28;
	bmp8[bitCountOffset] = 8;
	SelfTestDecode(bmp, bmp8, sizeof(bmp8), &pixels, &result);
	SelfTestCheck(test, result == ImageLoadResult_Unsupported, "An 8 bit BMP is unsupported by the reference loader");
}

// --- Image pyramid -----------------------------------------------------------------------------------------------------------

// xorshift32, deterministic so a failure can be reproduced
static uint32_t SelfTestNextRandom(uint32_t* state) {
	const uint32_t firstShift = 13;
	const uint32_t secondShift = 17;
	const uint32_t thirdShift = 5;
	uint32_t value = *state;
	value ^= value << firstShift;
	value ^= value >> secondShift;
	value ^= value << thirdShift;
	*state = value;
	return(value);
}

// Hard edges between black, white and random values, so the filters overshoot, saturate and meet every alpha case
static uint8_t SelfTestRandomSample(uint32_t* state) {
	const uint32_t choiceCount = 4;
	const uint8_t byteMaximum = 255;
	uint32_t choice = SelfTestNextRandom(state) % choiceCount;
	uint32_t value = SelfTestNextRandom(state);
	if (choice == 0) {
		return(0);
	}
	if (choice == 1) {
		return(byteMaximum);
	}
	return((uint8_t)value);
}

// Rows are stride bytes apart, padding bytes are random too and must never show up in a result
static void SelfTestFillRandomPicture(uint8_t* pixels, const uint32_t width, const uint32_t height, const uint32_t stride, const bool isOpaque, uint32_t* state) {
	const uint32_t bytesPerPixel = 4;
	const uint32_t alphaOffset = 3;
	const uint8_t opaqueAlpha = 255;
	for (uint32_t y = 0; y < height; ++y) {
		uint8_t* row = pixels + (size_t)y * stride;
		for (uint32_t byteIndex = 0; byteIndex < stride; ++byteIndex) {
			row[byteIndex] = SelfTestRandomSample(state);
		}
		if (isOpaque) {
			for (uint32_t x = 0; x < width; ++x) {
				row[x * bytesPerPixel + alphaOffset] = opaqueAlpha;
			}
		}
	}
}

static bool SelfTestAreLevelsEqual(const ImagePyramidLevel* a, const ImagePyramidLevel* b) {
	const uint32_t bytesPerPixel = 4;
	if (a->width != b->width || a->height != b->height) {
		return(false);
	}
	size_t rowBytes = (size_t)a->width * bytesPerPixel;
	for (uint32_t y = 0; y < a->height; ++y) {
		const uint8_t* rowA = a->pixels + (size_t)y * a->stride;
		const uint8_t* rowB = b->pixels + (size_t)y * b->stride;
		if (memcmp(rowA, rowB, rowBytes) != 0) {
			return(false);
		}
	}
	return(true);
}

static ImagePyramidSettings SelfTestPyramidSettings(const ImagePyramidRowFunctions* functions, const ImagePyramidKernel kernel) {
	ImagePyramidSettings result = fplZeroInit;
	result.functions = functions;
	result.kernel = kernel;
	return(result);
}

static void SelfTestPyramidTables(SelfTest* test) {
	test->groupName = "PyramidTables";
	ImagePyramidInitialize();
	const ImagePyramidTables* tables = ImagePyramidGetTables();
	const int32_t byteMaximum = 255;
	const int32_t linearMaximum = 32767;
	const int32_t weightOne = 16384;
	const int32_t firstTapOffset = -3;

	uint32_t roundTripFailures = 0;
	uint32_t alphaRoundTripFailures = 0;
	uint32_t monotonicFailures = 0;
	for (int32_t value = 0; value <= byteMaximum; ++value) {
		int16_t linear = tables->srgbToLinear[value];
		if (tables->linearToSRGB[linear] != value) {
			++roundTripFailures;
		}
		int32_t alpha = tables->alphaToLinear[value];
		int32_t alphaBack = (alpha * byteMaximum + linearMaximum / 2) / linearMaximum;
		if (alphaBack != value) {
			++alphaRoundTripFailures;
		}
		if (value > 0 && tables->srgbToLinear[value] <= tables->srgbToLinear[value - 1]) {
			++monotonicFailures;
		}
	}
	SelfTestCheck(test, roundTripFailures == 0, "Every sRGB value survives decode and encode");
	SelfTestCheck(test, alphaRoundTripFailures == 0, "Every alpha value survives decode and encode");
	SelfTestCheck(test, monotonicFailures == 0, "sRGB to linear rises strictly, dark values stay apart");
	SelfTestCheck(test, tables->srgbToLinear[0] == 0 && tables->srgbToLinear[byteMaximum] == linearMaximum, "sRGB 0 and 255 map to linear 0 and 32767");

	// Interior weights: symmetric and exactly one in Q14, for both kernels
	for (int kernelIndex = 0; kernelIndex < ImagePyramidKernel_Count; ++kernelIndex) {
		const ImagePyramidKernelDefinition* definition = ImagePyramidGetKernelDefinition((ImagePyramidKernel)kernelIndex);
		const int16_t* weights = tables->interiorWeights[kernelIndex];
		int32_t sum = 0;
		bool isSymmetric = true;
		for (uint32_t tap = 0; tap < IMAGE_PYRAMID_TAP_COUNT; ++tap) {
			sum += weights[tap];
			if (weights[tap] != weights[IMAGE_PYRAMID_TAP_COUNT - 1 - tap]) {
				isSymmetric = false;
			}
		}
		char text[256];
		fplStringFormat(text, fplArrayCount(text), "%s interior weights sum to 16384 and are symmetric", definition->name);
		SelfTestCheck(test, sum == weightOne && isSymmetric, text);
	}

	// Edge weights: taps inside the picture only, still exactly one, for every output of short and long axes
	const int32_t longestTestedLength = 40;
	uint32_t edgeSumFailures = 0;
	uint32_t edgeRangeFailures = 0;
	for (int kernelIndex = 0; kernelIndex < ImagePyramidKernel_Count; ++kernelIndex) {
		for (int32_t length = 1; length <= longestTestedLength; ++length) {
			int32_t outputCount = length / 2 + length % 2;
			for (int32_t output = 0; output < outputCount; ++output) {
				int32_t firstTap = 0;
				int16_t weights[IMAGE_PYRAMID_TAP_COUNT];
				uint32_t tapCount = ImagePyramidComputeTaps((ImagePyramidKernel)kernelIndex, output, length, &firstTap, weights);
				int32_t sum = 0;
				for (uint32_t tap = 0; tap < tapCount; ++tap) {
					sum += weights[tap];
				}
				int32_t endTap = firstTap + (int32_t)tapCount;
				int32_t expectedFirst = fplMax(2 * output + firstTapOffset, 0);
				if (sum != weightOne) {
					++edgeSumFailures;
				}
				if (tapCount == 0 || firstTap != expectedFirst || endTap > length) {
					++edgeRangeFailures;
				}
			}
		}
	}
	SelfTestCheck(test, edgeSumFailures == 0, "Edge weights sum to 16384 on every axis length 1..40");
	SelfTestCheck(test, edgeRangeFailures == 0, "Edge taps stay inside the picture");
}

static void SelfTestPyramidLevels(SelfTest* test) {
	test->groupName = "PyramidLevels";
	uint32_t photoLevelCount = ImagePyramidComputeLevelCount(4032, 3024);
	SelfTestCheck(test, photoLevelCount == 8, "4032x3024 has level 0 and 7 reduced levels");
	uint32_t smallestWidth = 0;
	uint32_t smallestHeight = 0;
	ImagePyramidComputeLevelSize(4032, 3024, 7, &smallestWidth, &smallestHeight);
	SelfTestCheck(test, smallestWidth == 32 && smallestHeight == 24, "4032x3024 level 7 is 32x24 (sizes round up)");
	uint32_t oddWidth = 0;
	uint32_t oddHeight = 0;
	ImagePyramidComputeLevelSize(1023, 767, 1, &oddWidth, &oddHeight);
	SelfTestCheck(test, oddWidth == 512 && oddHeight == 384, "1023x767 level 1 is 512x384, the last column and row stay inside");
	uint32_t tinyLevelCount = ImagePyramidComputeLevelCount(1, 1);
	uint32_t smallLevelCount = ImagePyramidComputeLevelCount(32, 32);
	uint32_t justAboveLevelCount = ImagePyramidComputeLevelCount(33, 1);
	SelfTestCheck(test, tinyLevelCount == 1 && smallLevelCount == 1 && justAboveLevelCount == 2, "Pictures of at most 32 pixels get no reduced level");
	uint32_t extremeLevelCount = ImagePyramidComputeLevelCount(40000, 64);
	uint32_t extremeWidth = 0;
	uint32_t extremeHeight = 0;
	ImagePyramidComputeLevelSize(40000, 64, 11, &extremeWidth, &extremeHeight);
	SelfTestCheck(test, extremeLevelCount == 12 && extremeWidth == 20 && extremeHeight == 1, "40000x64 ends at 20x1 after 11 reductions");

	// Flat pictures stay exactly flat through the whole pyramid: opaque, translucent and fully transparent
	const uint32_t flatWidth = 71;
	const uint32_t flatHeight = 45;
	const uint32_t bytesPerPixel = 4;
	const uint8_t flatColors[][4] = { { 128, 128, 128, 255 }, { 255, 0, 0, 255 }, { 200, 100, 50, 128 }, { 1, 2, 3, 254 }, { 90, 180, 30, 0 } };
	const uint8_t transparentPixel[4] = { 0, 0, 0, 0 };
	const ImagePyramidRowFunctions* scalar = ImagePyramidGetRowFunctions(SimdLevel_Scalar);
	ImagePyramidSettings settings = SelfTestPyramidSettings(scalar, ImagePyramidKernel_Mitchell);
	uint32_t flatStride = flatWidth * bytesPerPixel;
	uint8_t* flatPixels = (uint8_t*)malloc((size_t)flatStride * flatHeight);
	for (uint32_t colorIndex = 0; colorIndex < fplArrayCount(flatColors); ++colorIndex) {
		const uint8_t* color = flatColors[colorIndex];
		const uint8_t* expected = color[3] == 0 ? transparentPixel : color;
		for (uint32_t pixelIndex = 0; pixelIndex < flatWidth * flatHeight; ++pixelIndex) {
			memcpy(flatPixels + pixelIndex * bytesPerPixel, color, bytesPerPixel);
		}
		ImagePyramidLevel baseLevel = { flatPixels, flatWidth, flatHeight, flatStride };
		ImagePyramid pyramid;
		ImagePyramidResult result = ImagePyramidBuild(&baseLevel, &settings, &pyramid);
		uint32_t differentPixels = 0;
		for (uint32_t level = 1; level < pyramid.levelCount; ++level) {
			const ImagePyramidLevel* pyramidLevel = &pyramid.levels[level];
			for (uint32_t pixelIndex = 0; pixelIndex < pyramidLevel->width * pyramidLevel->height; ++pixelIndex) {
				if (memcmp(pyramidLevel->pixels + pixelIndex * bytesPerPixel, expected, bytesPerPixel) != 0) {
					++differentPixels;
				}
			}
		}
		char text[256];
		fplStringFormat(text, fplArrayCount(text), "Flat (%u, %u, %u, %u) stays flat on all %u levels", color[0], color[1], color[2], color[3], pyramid.levelCount);
		SelfTestCheck(test, result == ImagePyramidResult_Success && pyramid.levelCount > 1 && differentPixels == 0, text);
		ImagePyramidRelease(&pyramid);
	}
	free(flatPixels);

	// Cancel stops the build and leaves level 0 alone
	const uint32_t cancelWidth = 300;
	const uint32_t cancelHeight = 200;
	uint32_t cancelStride = cancelWidth * bytesPerPixel;
	uint8_t* cancelPixels = (uint8_t*)calloc((size_t)cancelStride * cancelHeight, 1);
	volatile bool isCanceled = true;
	ImagePyramidSettings cancelSettings = SelfTestPyramidSettings(scalar, ImagePyramidKernel_Mitchell);
	cancelSettings.cancelFlag = &isCanceled;
	ImagePyramidLevel cancelLevel = { cancelPixels, cancelWidth, cancelHeight, cancelStride };
	ImagePyramid canceledPyramid;
	ImagePyramidResult cancelResult = ImagePyramidBuild(&cancelLevel, &cancelSettings, &canceledPyramid);
	SelfTestCheck(test, cancelResult == ImagePyramidResult_Canceled && canceledPyramid.levelCount == 1 && canceledPyramid.memory == fpl_null, "A canceled build keeps only level 0");
	ImagePyramidRelease(&canceledPyramid);
	free(cancelPixels);
}

// Reduces the same picture with the scalar reference and with the level, true when every byte is equal
static bool SelfTestIsReductionBitIdentical(const ImagePyramidRowFunctions* functions, const ImagePyramidKernel kernel, const ImagePyramidLevel* source) {
	const uint32_t bytesPerPixel = 4;
	const ImagePyramidRowFunctions* scalar = ImagePyramidGetRowFunctions(SimdLevel_Scalar);
	ImagePyramidSettings scalarSettings = SelfTestPyramidSettings(scalar, kernel);
	ImagePyramidSettings levelSettings = SelfTestPyramidSettings(functions, kernel);
	uint32_t targetWidth = source->width / 2 + source->width % 2;
	uint32_t targetHeight = source->height / 2 + source->height % 2;
	uint32_t targetStride = targetWidth * bytesPerPixel;
	size_t targetBytes = (size_t)targetStride * targetHeight;
	uint8_t* scalarPixels = (uint8_t*)malloc(targetBytes);
	uint8_t* levelPixels = (uint8_t*)malloc(targetBytes);
	ImagePyramidLevel scalarTarget = { scalarPixels, targetWidth, targetHeight, targetStride };
	ImagePyramidLevel levelTarget = { levelPixels, targetWidth, targetHeight, targetStride };
	ImagePyramidResult scalarResult = ImagePyramidReduceHalf(source, &scalarTarget, &scalarSettings);
	ImagePyramidResult levelResult = ImagePyramidReduceHalf(source, &levelTarget, &levelSettings);
	bool result = scalarResult == ImagePyramidResult_Success && levelResult == ImagePyramidResult_Success && SelfTestAreLevelsEqual(&scalarTarget, &levelTarget);
	free(levelPixels);
	free(scalarPixels);
	return(result);
}

// Builds the whole pyramid with the scalar reference and with the level, true when every level is equal
static bool SelfTestIsPyramidBitIdentical(const ImagePyramidRowFunctions* functions, const ImagePyramidKernel kernel, const ImagePyramidLevel* baseLevel) {
	const ImagePyramidRowFunctions* scalar = ImagePyramidGetRowFunctions(SimdLevel_Scalar);
	ImagePyramidSettings scalarSettings = SelfTestPyramidSettings(scalar, kernel);
	ImagePyramidSettings levelSettings = SelfTestPyramidSettings(functions, kernel);
	ImagePyramid scalarPyramid;
	ImagePyramid levelPyramid;
	ImagePyramidResult scalarResult = ImagePyramidBuild(baseLevel, &scalarSettings, &scalarPyramid);
	ImagePyramidResult levelResult = ImagePyramidBuild(baseLevel, &levelSettings, &levelPyramid);
	bool result = scalarResult == ImagePyramidResult_Success && levelResult == ImagePyramidResult_Success && scalarPyramid.levelCount == levelPyramid.levelCount;
	for (uint32_t level = 1; result && level < scalarPyramid.levelCount; ++level) {
		result = SelfTestAreLevelsEqual(&scalarPyramid.levels[level], &levelPyramid.levels[level]);
	}
	ImagePyramidRelease(&levelPyramid);
	ImagePyramidRelease(&scalarPyramid);
	return(result);
}

// Every picture of the folder through every available SIMD level, compared with the scalar reference
static void SelfTestPyramidFolder(SelfTest* test, const char* folderPath, const fplCPUCapabilities* capabilities) {
	ImageLoaderRegistry registry;
	ImageLoaderRegistryInit(&registry);
	const ImageLoader* loaders[] = { ImageLoaderStbGet(), ImageLoaderPnmGet(), ImageLoaderBmpGet() };
	for (uint32_t loaderIndex = 0; loaderIndex < fplArrayCount(loaders); ++loaderIndex) {
		char message[IMAGE_LOADER_MESSAGE_SIZE];
		ImageLoaderRegistryAdd(&registry, loaders[loaderIndex], message, sizeof(message));
	}
	uint32_t pictureCount = 0;
	uint32_t failedCount = 0;
	fplFileEntry entry;
	for (bool hasEntry = fplDirectoryListBegin(folderPath, "*", &entry); hasEntry; hasEntry = fplDirectoryListNext(&entry)) {
		if (entry.type != fplFileEntryType_File) {
			continue;
		}
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, fplArrayCount(filePath), 2, folderPath, entry.name);
		const char* extension = fplExtractFileExtension(filePath);
		if (extension == fpl_null || !ImageLoaderRegistryIsKnownExtension(&registry, extension)) {
			continue;
		}
		ImageFileSource fileSource;
		if (!ImageFileSourceOpen(&fileSource, filePath, fpl_null, fpl_null, fpl_null)) {
			continue;
		}
		int32_t loaderEntry = -1;
		PictureInfo info;
		ImagePixels pixels = fplZeroInit;
		char message[IMAGE_LOADER_MESSAGE_SIZE];
		ImageLoadResult loadResult = ImageLoaderRegistryLoad(&registry, &fileSource.source, extension, -1, &loaderEntry, &info, &pixels, message, sizeof(message));
		ImageFileSourceClose(&fileSource);
		if (loadResult != ImageLoadResult_Success) {
			continue;
		}
		++pictureCount;
		ImagePyramidLevel baseLevel = { (uint8_t*)pixels.pixels, pixels.width, pixels.height, pixels.stride };
		for (int32_t level = SimdLevel_Scalar + 1; level < SimdLevel_Count; ++level) {
			if (!ImagePyramidIsSimdLevelAvailable(capabilities, (SimdLevel)level)) {
				continue;
			}
			const ImagePyramidRowFunctions* functions = ImagePyramidGetRowFunctions((SimdLevel)level);
			for (int kernelIndex = 0; kernelIndex < ImagePyramidKernel_Count; ++kernelIndex) {
				if (!SelfTestIsPyramidBitIdentical(functions, (ImagePyramidKernel)kernelIndex, &baseLevel)) {
					const ImagePyramidKernelDefinition* kernelDefinition = ImagePyramidGetKernelDefinition((ImagePyramidKernel)kernelIndex);
					++failedCount;
					fplConsoleFormatError("[%s] %s differs from scalar on '%s' (%s)\n", test->groupName, functions->name, entry.name, kernelDefinition->key);
				}
			}
		}
		ImageLoaderRegistryReleasePixels(&registry, loaderEntry, &pixels);
	}
	ImageLoaderRegistryRelease(&registry);
	char text[256];
	fplStringFormat(text, fplArrayCount(text), "All %u test pictures of '%s' are bit identical on every SIMD level", pictureCount, folderPath);
	SelfTestCheck(test, pictureCount > 0 && failedCount == 0, text);
}

static void SelfTestPyramidSimd(SelfTest* test, const char* imageFolder) {
	test->groupName = "PyramidSimd";
	const uint32_t bytesPerPixel = 4;
	const uint32_t largestTestedSize = 130;
	const uint32_t randomSizeCount = 300;
	const uint32_t rowPaddingBytes = 12;
	const uint32_t randomSeed = 0x2545F491;
	// Failures listed one by one per SIMD level, the rest is only counted
	const uint32_t largestReportedFailures = 5;
	const uint32_t fixedHeights[] = { 1, 2, 3, 4, 5, 7, 8, 9, 16, 17, 33, 130 };
	fplCPUCapabilities capabilities = fplZeroInit;
	fplCPUGetCapabilities(&capabilities);

	// Falling back: a level that is not available picks the next lower one, scalar is always there
	const ImagePyramidRowFunctions* best = ImagePyramidSelectRowFunctions(&capabilities, SimdLevel_Best);
	const ImagePyramidRowFunctions* scalar = ImagePyramidSelectRowFunctions(&capabilities, SimdLevel_Scalar);
	SelfTestCheck(test, best != fpl_null && scalar != fpl_null && scalar->level == SimdLevel_Scalar, "Best and scalar row functions exist");
	bool isNeonAvailable = ImagePyramidIsSimdLevelAvailable(&capabilities, SimdLevel_ARM_NEON);
	const ImagePyramidRowFunctions* requestedNeon = ImagePyramidSelectRowFunctions(&capabilities, SimdLevel_ARM_NEON);
	SelfTestCheck(test, isNeonAvailable || requestedNeon == best, "NEON without NEON falls back to the best available level");
	for (int32_t level = SimdLevel_Scalar; level < SimdLevel_Count; ++level) {
		const ImagePyramidRowFunctions* selected = ImagePyramidSelectRowFunctions(&capabilities, (SimdLevel)level);
		bool isAvailable = ImagePyramidIsSimdLevelAvailable(&capabilities, (SimdLevel)level);
		const char* requestedName = ImagePyramidGetSimdLevelName((SimdLevel)level);
		char text[256];
		fplStringFormat(text, fplArrayCount(text), "Requesting %s gives %s", requestedName, selected->name);
		SelfTestCheck(test, isAvailable ? selected->level == (SimdLevel)level : selected->level < (SimdLevel)level, text);
	}
	char levelsText[256] = fplZeroInit;
	for (int32_t level = SimdLevel_Scalar; level < SimdLevel_Count; ++level) {
		if (ImagePyramidIsSimdLevelAvailable(&capabilities, (SimdLevel)level)) {
			const char* levelName = ImagePyramidGetSimdLevelName((SimdLevel)level);
			fplStringAppend(levelName, levelsText, fplArrayCount(levelsText));
			fplStringAppend(" ", levelsText, fplArrayCount(levelsText));
		}
	}
	fplConsoleFormatOut("[%s] Available SIMD levels: %s\n", test->groupName, levelsText);

	// Random pictures: every width up to 130 with a set of heights, plus random sizes, with padded rows, both kernels
	uint32_t randomState = randomSeed;
	size_t largestStride = (size_t)largestTestedSize * bytesPerPixel + rowPaddingBytes;
	uint8_t* pixels = (uint8_t*)malloc(largestStride * largestTestedSize);
	for (int32_t level = SimdLevel_Scalar + 1; level < SimdLevel_Count; ++level) {
		if (!ImagePyramidIsSimdLevelAvailable(&capabilities, (SimdLevel)level)) {
			continue;
		}
		const ImagePyramidRowFunctions* functions = ImagePyramidGetRowFunctions((SimdLevel)level);
		uint32_t caseCount = 0;
		uint32_t failedCount = 0;
		uint32_t sizeCount = largestTestedSize * fplArrayCount(fixedHeights) + randomSizeCount;
		for (uint32_t sizeIndex = 0; sizeIndex < sizeCount; ++sizeIndex) {
			uint32_t width;
			uint32_t height;
			if (sizeIndex < largestTestedSize * fplArrayCount(fixedHeights)) {
				width = sizeIndex % largestTestedSize + 1;
				height = fixedHeights[sizeIndex / largestTestedSize];
			} else {
				width = SelfTestNextRandom(&randomState) % largestTestedSize + 1;
				height = SelfTestNextRandom(&randomState) % largestTestedSize + 1;
			}
			bool isOpaque = (sizeIndex % 3) == 0;
			bool isPadded = (sizeIndex % 2) == 0;
			uint32_t stride = width * bytesPerPixel + (isPadded ? rowPaddingBytes : 0);
			SelfTestFillRandomPicture(pixels, width, height, stride, isOpaque, &randomState);
			ImagePyramidLevel source = { pixels, width, height, stride };
			ImagePyramidKernel kernel = (ImagePyramidKernel)(sizeIndex % ImagePyramidKernel_Count);
			++caseCount;
			if (!SelfTestIsReductionBitIdentical(functions, kernel, &source)) {
				if (failedCount < largestReportedFailures) {
					const ImagePyramidKernelDefinition* kernelDefinition = ImagePyramidGetKernelDefinition(kernel);
					fplConsoleFormatError("[%s] %s differs from scalar at %u x %u (stride %u, %s)\n", test->groupName, functions->name, width, height, stride, kernelDefinition->key);
				}
				++failedCount;
			}
		}
		char text[256];
		fplStringFormat(text, fplArrayCount(text), "%s reduces %u random pictures bit identical to scalar", functions->name, caseCount);
		SelfTestCheck(test, failedCount == 0, text);

		// Whole pyramids of larger pictures reach the wide SIMD loops on several levels
		const uint32_t largeSizes[][2] = { { 1000, 37 }, { 97, 777 }, { 1023, 767 } };
		uint32_t largeFailures = 0;
		for (uint32_t largeIndex = 0; largeIndex < fplArrayCount(largeSizes); ++largeIndex) {
			uint32_t largeWidth = largeSizes[largeIndex][0];
			uint32_t largeHeight = largeSizes[largeIndex][1];
			uint32_t largeStride = largeWidth * bytesPerPixel;
			uint8_t* largePixels = (uint8_t*)malloc((size_t)largeStride * largeHeight);
			SelfTestFillRandomPicture(largePixels, largeWidth, largeHeight, largeStride, largeIndex == 0, &randomState);
			ImagePyramidLevel largeLevel = { largePixels, largeWidth, largeHeight, largeStride };
			for (int kernelIndex = 0; kernelIndex < ImagePyramidKernel_Count; ++kernelIndex) {
				if (!SelfTestIsPyramidBitIdentical(functions, (ImagePyramidKernel)kernelIndex, &largeLevel)) {
					++largeFailures;
				}
			}
			free(largePixels);
		}
		fplStringFormat(text, fplArrayCount(text), "%s builds whole pyramids of 1000x37, 97x777 and 1023x767 bit identical to scalar", functions->name);
		SelfTestCheck(test, largeFailures == 0, text);
	}
	free(pixels);

	if (imageFolder != fpl_null) {
		SelfTestPyramidFolder(test, imageFolder, &capabilities);
	}
}

static void SelfTestSourceLevel(SelfTest* test) {
	test->groupName = "SourceLevel";
	const uint32_t photoLevelCount = 8;

	// The scale relative to the source level lies in (1/16, 1/8], level 0 serves everything above 1/16
	typedef struct SourceLevelCase {
		float scale;
		uint32_t levelCount;
		uint32_t firstLevel;
		uint32_t expectedLevel;
		const char* description;
	} SourceLevelCase;
	const SourceLevelCase cases[] = {
		{ 1.0f, photoLevelCount, 0, 0, "Scale 1 reads level 0" },
		{ 0.238f, photoLevelCount, 0, 0, "Scale 0.238 (a photo fitted into 1280x720) reads level 0" },
		{ 0.125f, photoLevelCount, 0, 0, "Scale 0.125 reads level 0" },
		{ 0.1f, photoLevelCount, 0, 0, "Scale 0.1 reads level 0" },
		{ 0.0625f, photoLevelCount, 0, 1, "Scale 0.0625 reads level 1 at 0.125" },
		{ 0.05f, photoLevelCount, 0, 1, "Scale 0.05 reads level 1" },
		{ 0.03f, photoLevelCount, 0, 2, "Scale 0.03 reads level 2" },
		{ 0.001f, 3, 0, 2, "A tiny scale reads the last level" },
		{ 1.0f, photoLevelCount, 1, 1, "Scale 1 reads level 1 when level 0 has no texture" },
		{ 0.03f, photoLevelCount, 3, 3, "Scale 0.03 reads level 3 when the levels before have no texture" },
		{ 0.0f, photoLevelCount, 0, 0, "No scale reads level 0" },
		{ 0.03f, 1, 0, 0, "Without reduced levels everything reads level 0" },
	};
	for (uint32_t caseIndex = 0; caseIndex < fplArrayCount(cases); ++caseIndex) {
		const SourceLevelCase* sourceCase = &cases[caseIndex];
		uint32_t level = ComputeViewSourceLevel(sourceCase->scale, sourceCase->levelCount, sourceCase->firstLevel);
		SelfTestCheck(test, level == sourceCase->expectedLevel, sourceCase->description);
	}
	const float firstSweepScale = 0.0002f;
	const float sweepStep = 1.013f;
	const float smallestLevelScale = 0.0625f;
	const float largestLevelScale = 0.125f;
	uint32_t rangeFailures = 0;
	for (float scale = firstSweepScale; scale <= 1.0f; scale *= sweepStep) {
		uint32_t level = ComputeViewSourceLevel(scale, IMAGE_PYRAMID_MAX_LEVELS, 0);
		float levelScale = ldexpf(scale, (int)level);
		bool isInRange = level == 0 ? scale > smallestLevelScale : (levelScale > smallestLevelScale && levelScale <= largestLevelScale);
		if (!isInRange) {
			++rangeFailures;
		}
	}
	SelfTestCheck(test, rangeFailures == 0, "Sweep: the scale relative to the source level stays in (1/16, 1/8]");

	// 5x3 stored, level 1 is 3x2 and covers 6x4 picture pixels: its last column and row lie half on the picture.
	// A mirrored axis shows that pixel first, shifted by 3 - 5 / 2 = 0.5 or 2 - 3 / 2 = 0.5.
	const uint32_t firstOrientation = 1;
	const uint32_t lastOrientation = 8;
	const float half = 0.5f;
	const float whole = 1.0f;
	const bool isColumnMirrored[] = { false, false, true, true, false, false, true, true, false };
	const bool isRowMirrored[] = { false, false, false, true, true, false, false, true, true };
	ViewSize storedSize = fplStructInit(ViewSize, 5, 3);
	ViewSize storedLevelSize = fplStructInit(ViewSize, 3, 2);
	uint32_t placementFailures = 0;
	uint32_t levelZeroFailures = 0;
	for (uint32_t orientation = firstOrientation; orientation <= lastOrientation; ++orientation) {
		ViewLevelPlacement placement = ComputeViewLevelPlacement(orientation, storedSize, storedLevelSize, 1);
		float expectedOffsetU = isColumnMirrored[orientation] ? half : 0.0f;
		float expectedOffsetV = isRowMirrored[orientation] ? half : 0.0f;
		float expectedFirstU = isColumnMirrored[orientation] ? half : whole;
		float expectedLastU = isColumnMirrored[orientation] ? whole : half;
		float expectedFirstV = isRowMirrored[orientation] ? half : whole;
		float expectedLastV = isRowMirrored[orientation] ? whole : half;
		bool isOffsetRight = fabsf(placement.offsetU - expectedOffsetU) <= SelfTestFloatTolerance && fabsf(placement.offsetV - expectedOffsetV) <= SelfTestFloatTolerance;
		bool isColumnCoverageRight = fabsf(placement.firstCoverageU - expectedFirstU) <= SelfTestFloatTolerance && fabsf(placement.lastCoverageU - expectedLastU) <= SelfTestFloatTolerance;
		bool isRowCoverageRight = fabsf(placement.firstCoverageV - expectedFirstV) <= SelfTestFloatTolerance && fabsf(placement.lastCoverageV - expectedLastV) <= SelfTestFloatTolerance;
		if (!isOffsetRight || !isColumnCoverageRight || !isRowCoverageRight) {
			++placementFailures;
			fplConsoleFormatError("[%s] orientation %u: offset (%f, %f), coverage u (%f, %f), v (%f, %f)\n", test->groupName, orientation, placement.offsetU, placement.offsetV, placement.firstCoverageU, placement.lastCoverageU, placement.firstCoverageV, placement.lastCoverageV);
		}
		ViewLevelPlacement levelZero = ComputeViewLevelPlacement(orientation, storedSize, storedSize, 0);
		bool isLevelZeroWhole = levelZero.offsetU == 0.0f && levelZero.offsetV == 0.0f && levelZero.firstCoverageU == whole && levelZero.lastCoverageU == whole && levelZero.firstCoverageV == whole && levelZero.lastCoverageV == whole;
		if (!isLevelZeroWhole) {
			++levelZeroFailures;
		}
	}
	SelfTestCheck(test, placementFailures == 0, "An odd sized level is shifted onto the picture and its overhanging edge pixel is covered by half, for all 8 orientations");
	SelfTestCheck(test, levelZeroFailures == 0, "Level 0 needs no shift and covers the picture completely");
}

// imageFolder is optional: when set, every picture in it is also reduced on every SIMD level and compared
static int RunSelfTest(const char* imageFolder) {
	SelfTest test = fplZeroInit;
	ImagePyramidInitialize();
	SelfTestViewTransform(&test);
	SelfTestResample(&test);
	SelfTestOrientation(&test);
	SelfTestLoaderIds(&test);
	SelfTestLoaderSelection(&test);
	SelfTestExif(&test);
	SelfTestLoaders(&test);
	SelfTestPyramidTables(&test);
	SelfTestPyramidLevels(&test);
	SelfTestPyramidSimd(&test, imageFolder);
	SelfTestSourceLevel(&test);
	fplConsoleFormatOut("Self test: %u checks, %u failed\n", test.checkCount, test.failedCount);
	int result = test.failedCount == 0 ? 0 : 1;
	return(result);
}

#endif // SELFTEST_H
