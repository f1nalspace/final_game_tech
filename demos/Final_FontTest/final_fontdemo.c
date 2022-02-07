#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FNT_IMPLEMENTATION
#include <final_font.h>

#include "font_avril_sans_regular.h"

typedef struct UnicodeRange {
	uint32_t from;
	uint32_t to;
} UnicodeRange;

// https://stackoverflow.com/a/30200250
// http://www.localizingjapan.com/blog/2012/01/20/regular-expressions-for-japanese-text/
static UnicodeRange g__unicodeRanges[] = {
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

			fntFontInfo fontInfo = fplZeroInit;
			if (fntLoadFontInfo(&fontData, &fontInfo, 0, 120.0f)) {
				fntFontAtlas atlas = fplZeroInit;
				if (fntInitFontAtlas(&fontInfo, &atlas)) {
					fntFontContext *ctx = fntCreateFontContext(&fontData, &fontInfo, 512);
					if (ctx != fpl_null) {

						for (uint32_t i = 0; i < fplArrayCount(g__unicodeRanges); ++i) {
							uint32_t range = g__unicodeRanges[i].to - g__unicodeRanges[i].from;
							fntAddToFontAtlas(ctx, &atlas, g__unicodeRanges[i].from, range);
						}

						// @TODO(final): This is really slow, make this an offline tool!
						//fntComputeAtlasKernings(ctx, &atlas);

						fntReleaseFontContext(ctx);
					}

					// UTF8-Encode (https://onlineunicodetools.com/convert-unicode-to-utf8)
					const char *helloWorldText = "Hello World!";
					const char japAnimeText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0 }; // A ni me, 3 characters
					const char japAnimeAndKanaText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0x20, 0x61, 0x6e, 0x69, 0x6d, 0x65, 0 }; // A ni me anime, 9 characters

					size_t quadCount0 = fntGetQuadCountFromUTF8(&atlas, helloWorldText);
					size_t quadCount1 = fntGetQuadCountFromUTF8(&atlas, japAnimeText);
					size_t quadCount2 = fntGetQuadCountFromUTF8(&atlas, japAnimeAndKanaText);

					fntFontQuad fontQuads[16];
					fntVec2 quadsSize;
					bool r;

					r = fntComputeQuadsFromUTF8(&atlas, &fontInfo, helloWorldText, 20.0f, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);
					r = fntComputeQuadsFromUTF8(&atlas, &fontInfo, japAnimeText, 20.0f, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);
					r = fntComputeQuadsFromUTF8(&atlas, &fontInfo, japAnimeAndKanaText, 20.0f, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsSize);

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

					fntFreeFontAtlas(&atlas);
				}
				fntFreeFontInfo(&fontInfo);
			}
		}

		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {
			}
		}

		fglUnloadOpenGL();
	}
	fplPlatformRelease();
	return (0);
}