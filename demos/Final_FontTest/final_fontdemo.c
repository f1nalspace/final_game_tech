#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FNT_IMPLEMENTATION
#include <final_font.h>

#include "font_avril_sans_regular.h"

typedef struct CodePointRange {
	uint16_t from;
	uint16_t to;
} CodePointRange;

// https://stackoverflow.com/a/30200250
// http://www.localizingjapan.com/blog/2012/01/20/regular-expressions-for-japanese-text/
static CodePointRange g__unicodeRanges[] = {
	{33, 126},			// ASCII
	//{0x3000, 0x303f},	// Japanese-style punctuation
	//{0x3040, 0x309f},	// Hiragana
	//{0x30a0, 0x30ff},	// Katakana
	//{0xff00, 0xffef},	// Full-width roman characters and half-width katakana
	//{0x4e00, 0x9faf},	// CJK unifed ideographs - Common and uncommon kanji
};

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Final Demo - Fonts", settings.window.title, sizeof(settings.window.title));
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			fntFontData fontData = fplZeroInit;
			fontData.size = fontAvrilSansRegularLength;
			fontData.data = fontAvrilSansRegularPtr;
			fontData.name = fontAvrilSansRegularName;
			fontData.index = 0;

			const fntFontSize fontSize = { 128.0f };

			fntFontAtlas atlas = fplZeroInit;
			if (fntInitFontAtlas(&atlas, &fontData, fontSize)) {
				fntFontContext *ctx = fntCreateFontContext(512);
				if (ctx != NULL) {
					for (uint32_t i = 0; i < fplArrayCount(g__unicodeRanges); ++i) {
						uint16_t range = g__unicodeRanges[i].to - g__unicodeRanges[i].from;
						fntCodePoint from = { g__unicodeRanges[i].from };
						fntCodePoint end = { g__unicodeRanges[i].to };
						fntAddToFontAtlas(ctx, &atlas, &fontData, fontSize, from, end);
					}
					fntReleaseFontContext(ctx);
				}

				// UTF8-Encode (https://onlineunicodetools.com/convert-unicode-to-utf8)
				const char *helloWorldText = "Hello World!";
				const char *testText = "Five Wax Quacking Zephyrs";
				const char japAnimeText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0 }; // A ni me, 3 characters
				const char japAnimeAndKanaText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0x20, 0x61, 0x6e, 0x69, 0x6d, 0x65, 0 }; // A ni me anime, 9 characters

				const float targetCharHeight = 20.0f;

				fntFontQuad fontQuads[32];
				fntVec2 quadsSize;
				size_t quadCount;
				bool r;

				quadCount = fntGetQuadCountFromUTF8(helloWorldText);
				fplAssert(quadCount == 12);
				r = fntComputeQuadsFromUTF8(&atlas, helloWorldText, fontSize, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);

				quadCount = fntGetQuadCountFromUTF8(testText);
				fplAssert(quadCount == 25);
				r = fntComputeQuadsFromUTF8(&atlas, testText, fontSize, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);

				quadCount = fntGetQuadCountFromUTF8(japAnimeText);
				fplAssert(quadCount == 3);
				r = fntComputeQuadsFromUTF8(&atlas, japAnimeText, fontSize, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);

				quadCount = fntGetQuadCountFromUTF8(japAnimeAndKanaText);
				fplAssert(quadCount == 9);
				r = fntComputeQuadsFromUTF8(&atlas, japAnimeAndKanaText, fontSize, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);

				char homePath[FPL_MAX_PATH_LENGTH];
				fplGetHomePath(homePath, sizeof(homePath));

				char bitmapFilePath[FPL_MAX_PATH_LENGTH];

				char bitmapFilename[100];
				for (uint32_t bitmapIndex = 0; bitmapIndex < atlas.bitmapCount; ++bitmapIndex) {
					const fntBitmap *bitmap = atlas.bitmaps + bitmapIndex;
					fplFormatString(bitmapFilename, sizeof(bitmapFilename), "font_bitmap%lu.bmp", bitmapIndex);
					fplPathCombine(bitmapFilePath, sizeof(bitmapFilePath), 3, homePath, "Downloads", bitmapFilename);
					fntSaveBitmapToFile(bitmap, bitmapFilePath);
				}

				while (fplWindowUpdate()) {
					fplEvent ev;
					while (fplPollEvent(&ev)) {
					}
				}

				fntFreeFontAtlas(&atlas);
			}
			fglUnloadOpenGL();
		}
		fplPlatformRelease();
	}
	return (0);
}