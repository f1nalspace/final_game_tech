#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FNT_IMPLEMENTATION
#include <final_font.h>

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Final Demo - Fonts", settings.window.title, sizeof(settings.window.title));
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			const char *fontFilePath = "c:/windows/fonts/l_10646.ttf";
			const char *fontName = "Lucida Sans Unicode";
			if (fplFileExists(fontFilePath)) {
				fplFileHandle file;
				if (fplFileOpenBinary(fontFilePath, &file)) {
					size_t size = fplFileGetSizeFromHandle(&file);
					uint8_t *data = (uint8_t *)malloc(size);
					fplFileReadBlock(&file, size, data, size);
					fplFileClose(&file);

					fntFontData fontData = fplZeroInit;
					fontData.size = size;
					fontData.data = data;
					fontData.name = fontName;

					fntFontInfo fontInfo = fplZeroInit;
					if (fntLoadFontInfo(&fontData, &fontInfo, 0, 80.0f)) {
						fntFontAtlas atlas = fplZeroInit;
						if (fntInitFontAtlas(&fontInfo, &atlas)) {
							fntFontContext *ctx = fntCreateFontContext(&fontData, &fontInfo, 512);
							if (ctx != fpl_null) {
								fntAddToFontAtlas(ctx, &atlas, 33, 133);
								fntComputeAtlasKernings(ctx, &atlas);
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

					free(data);
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
	}
	return (0);
}