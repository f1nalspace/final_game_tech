#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FNT_IMPLEMENTATION
#include <final_font.h>

#include "font_avril_sans_regular.h"
#include "font_sulfur_point_regular.h"

typedef struct CodePointRange {
	uint16_t from;
	uint16_t to;
} CodePointRange;

// https://stackoverflow.com/a/30200250
// http://www.localizingjapan.com/blog/2012/01/20/regular-expressions-for-japanese-text/
static CodePointRange g__unicodeRanges[] = {
	{33, 126},			// ASCII
	{161, 255},			// Extended ASCII
	{0x3000, 0x303f},	// Japanese-style punctuation
	{0x3040, 0x309f},	// Hiragana
	{0x30a0, 0x30ff},	// Katakana
	{0xff00, 0xffef},	// Full-width roman characters and half-width katakana
	//{0x4e00, 0x9faf},	// CJK unifed ideographs - Common and uncommon kanji
};

static GLuint CreateRGBATextureFromAlpha(const uint8_t *alphaPixels, const uint32_t width, const uint32_t height) {
	GLuint result = 0;
	uint32_t *rgbaPixels = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
	if (rgbaPixels != NULL) {
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				uint8_t alpha = alphaPixels[y * width + x];
				uint8_t r = alpha;
				uint8_t g = alpha;
				uint8_t b = alpha;
				uint8_t a = 255;
				uint32_t rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
				rgbaPixels[y * width + x] = rgba;
			}
		}

		glGenTextures(1, &result);
		glBindTexture(GL_TEXTURE_2D, result);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		free(rgbaPixels);
	}
	return(result);
}

static fntFontData LoadFontFromFile(const char *filePath) {
	fntFontData result = fplZeroInit;

	fplFileHandle file;
	if (fplFileOpenBinary(filePath, &file)) {
		size_t len = fplFileGetSizeFromHandle(&file);
		uint8_t *data = (uint8_t *)malloc(len);
		fplFileReadBlock(&file, len, data, len);
		fplFileClose(&file);

		result.name = filePath;
		result.index = 0;
		result.data = data;
		result.size = len;
	}
	return(result);
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Final Demo - Fonts", settings.window.title, sizeof(settings.window.title));
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			fntFontData fontData = fplZeroInit;

			// Load unicode font from downloads folders (Due to legal limitations, the font is not included)
#if 1
			{
				size_t len = fplGetHomePath(fpl_null, 0) + 1;
				char *homePath = fplStackAllocate(sizeof(char) * len);
				fplGetHomePath(homePath, len);

				len = fplPathCombine(fpl_null, 0, 3, homePath, "Downloads", "arial-unicode-ms.ttf") + 1;
				char *fontFilePath = fplStackAllocate(sizeof(char) * len);
				fplPathCombine(fontFilePath, len, 3, homePath, "Downloads", "arial-unicode-ms.ttf");

				fontData = LoadFontFromFile(fontFilePath);
			}
#endif

			// Load arial font windows folder
#if 0
#if defined(FPL_PLATFORM_WINDOWS)
			{
				size_t len = GetWindowsDirectoryA(fpl_null, 0) + 1;
				char *winPath = fplStackAllocate(sizeof(char) * len);
				GetWindowsDirectoryA(winPath, len);

				len = fplPathCombine(fpl_null, 0, 3, winPath, "fonts", "arial.ttf") + 1;
				char *fontFilePath = fplStackAllocate(sizeof(char) * len);
				fplPathCombine(fontFilePath, len, 3, winPath, "fonts", "arial.ttf");

				fontData = LoadFontFromFile(fontFilePath);
			}
#endif
#endif

			// Use Sulphur Point Regular font
#if 0
			fontData.size = fontSulphurPointRegularSize;
			fontData.data = fontSulphurPointRegularData;
			fontData.name = fontSulphurPointRegularName;
			fontData.index = 0;
#endif

			// Use Avril Sans Regular font
#if 0
			fontData.size = fontAvrilSansRegularLength;
			fontData.data = fontAvrilSansRegularData;
			fontData.name = fontAvrilSansRegularName;
			fontData.index = 0;
#endif

			const uint32_t maxAtlasSize = 1024;

			const fntFontSize fontSize = fntCreateFontSize(128.0f);

			fntFontAtlas atlas = fplZeroInit;
			if (fntInitFontAtlas(&atlas, &fontData, fontSize)) {
				fntFontContext *ctx = fntCreateFontContext(maxAtlasSize);
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
				const char *fiveWaxText = "Five Wax Quacking Zephyrs";
				const char *brownFoxText = "The quick brown fox jumps over the lazy dog";
				const char japAnimeText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0 }; // A ni me, 3 characters
				const char japAnimeAndKanaText[] = { 0xe3, 0x82, 0xa2, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0xa1, 0x20, 0x61, 0x6e, 0x69, 0x6d, 0x65, 0 }; // A ni me anime, 9 characters

				const float targetCharHeight = 20.0f;

				fntFontQuad fontQuads[64];
				fntBounds quadsBounds;
				size_t quadCount;

#if 1
				{

					bool r;

					quadCount = fntGetQuadCountFromUTF8(helloWorldText);
					fplAssert(quadCount == 12);
					r = fntComputeQuadsFromUTF8(&atlas, helloWorldText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds, NULL, NULL);
					fplAssert(r == true);

					quadCount = fntGetQuadCountFromUTF8(japAnimeText);
					fplAssert(quadCount == 3);
					r = fntComputeQuadsFromUTF8(&atlas, japAnimeText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds, NULL, NULL);
					fplAssert(r == true);

					quadCount = fntGetQuadCountFromUTF8(japAnimeAndKanaText);
					fplAssert(quadCount == 9);
					r = fntComputeQuadsFromUTF8(&atlas, japAnimeAndKanaText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds, NULL, NULL);
					fplAssert(r == true);

					quadCount = fntGetQuadCountFromUTF8(fiveWaxText);
					fplAssert(quadCount == 25);
					r = fntComputeQuadsFromUTF8(&atlas, fiveWaxText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds, NULL, NULL);
					fplAssert(r == true);
				}
#endif

				char homePath[FPL_MAX_PATH_LENGTH];
				fplGetHomePath(homePath, sizeof(homePath));

				char bitmapFilePath[FPL_MAX_PATH_LENGTH];

				char bitmapFilename[100];
				for (uint32_t bitmapIndex = 0; bitmapIndex < atlas.bitmapCount; ++bitmapIndex) {
					const fntBitmap *bitmap = atlas.bitmaps + bitmapIndex;
					fplStringFormat(bitmapFilename, sizeof(bitmapFilename), "font_bitmap%lu.bmp", bitmapIndex);
					fplPathCombine(bitmapFilePath, sizeof(bitmapFilePath), 3, homePath, "Downloads", bitmapFilename);
					fntSaveBitmapToFile(bitmap, bitmapFilePath);
				}

				GLuint fontTextures[8] = fplZeroInit;

				for (uint32_t bitmapIndex = 0; bitmapIndex < atlas.bitmapCount; ++bitmapIndex) {
					fontTextures[bitmapIndex] = CreateRGBATextureFromAlpha(atlas.bitmaps[bitmapIndex].pixels, atlas.bitmaps[bitmapIndex].width, atlas.bitmaps[bitmapIndex].height);
				}

				glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
				glMatrixMode(GL_MODELVIEW);

				bool topDown = false;
				bool withKerning = true;

				while (fplWindowUpdate()) {
					fplEvent ev;
					while (fplPollEvent(&ev)) {
						switch (ev.type) {
							case fplEventType_Keyboard:
							{
								switch (ev.keyboard.type) {
									case fplKeyboardEventType_Button:
									{
										if (ev.keyboard.buttonState == fplButtonState_Release) {
											if (ev.keyboard.mappedKey == fplKey_Space) {
												topDown = !topDown;
											} else if (ev.keyboard.mappedKey == fplKey_K) {
												withKerning = !withKerning;
											}
										}
									} break;
								}
							} break;
						}
					}

					fplWindowSize winSize;
					fplGetWindowSize(&winSize);

					glViewport(0, 0, winSize.width, winSize.height);

					float w = (float)winSize.width * 0.5f;
					float h = (float)winSize.height * 0.5f;

					glLoadIdentity();
					if (topDown) {
						glOrtho(-w, w, h, -h, -1.0f, 1.0f);
					} else {
						glOrtho(-w, w, -h, h, -1.0f, 1.0f);
					}
					glScalef(1.0f, 1.0f, 1.0f);

					glClear(GL_COLOR_BUFFER_BIT);

					glColor3f(1.0f, 1.0f, 1.0f);
					glBegin(GL_LINES);
					glVertex2f(-w, 0.0f);
					glVertex2f(w, 0.0f);
					glVertex2f(0.0f, -h);
					glVertex2f(0.0f, h);
					glEnd();

					const char *text = japAnimeText;

					fntComputeQuadsFlags flags = fntComputeQuadsFlags_None;
					if (!topDown) {
						flags |= fntComputeQuadsFlags_Cartesian;
					}
					if (!withKerning) {
						flags |= fntComputeQuadsFlags_WithoutKerning;
					}

					//flags |= fntComputeQuadsFlags_GlyphAdvancement;
					//flags |= fntComputeQuadsFlags_AlignToDescent;
					//flags |= fntComputeQuadsFlags_AlignLeft;

					quadCount = fntGetQuadCountFromUTF8(text);

					size_t maxQuadCount = fplArrayCount(fontQuads);
					fplAssert(maxQuadCount >= quadCount);

					float textX = 0.0f, textY = 0.0f;
#if 0
					// Center
					fntVec2 textSize = fntComputeTextSizeFromUTF8(&atlas, text, fontSize.f32, flags);
					textX -= textSize.w * 0.5f;
					textY -= textSize.h * 0.5f;
#endif
					float scaledAscent = 0.0f;
					float scaledDescent = 0.0f;
					float scaledLineGap = 0.0f;
					fntGetFontMetrics(&atlas, fontSize.value, &scaledAscent, &scaledDescent, &scaledLineGap);

					float baseline = -scaledDescent;

					uint32_t lineCount = 0;
					float baselineOffset = 0;
					if (fntComputeQuadsFromUTF8(&atlas, text, fontSize.value, flags, fplArrayCount(fontQuads), fontQuads, &quadsBounds, &lineCount, &baselineOffset)) {
						float lineHeight = scaledAscent - scaledDescent;

						float boundsWidth = quadsBounds.right - quadsBounds.left;

						textX -= boundsWidth * 0.5f;

						// Line Top
						glColor3f(0.0f, 0.0f, 1.0f);
						glBegin(GL_LINE_LOOP);
						glVertex2f(textX + quadsBounds.right, textY + 0.0f);
						glVertex2f(textX + quadsBounds.left, textY + 0.0f);
						glVertex2f(textX + quadsBounds.left, textY + lineHeight);
						glVertex2f(textX + quadsBounds.right, textY + lineHeight);
						glEnd();

						// Ascent
						glLineWidth(2.0f);
						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(textX + quadsBounds.right + boundsWidth * 0.25f, textY + baseline + scaledAscent);
						glVertex2f(textX + quadsBounds.left - boundsWidth * 0.25f, textY + baseline + scaledAscent);
						glEnd();
						glLineWidth(1.0f);

						// Baseline
						glLineWidth(2.0f);
						glColor3f(1.0f, 0.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(textX + quadsBounds.right + boundsWidth * 0.25f, textY + baseline);
						glVertex2f(textX + quadsBounds.left - boundsWidth * 0.25f, textY + baseline);
						glEnd();
						glLineWidth(1.0f);

						// Descent
						glLineWidth(2.0f);
						glColor3f(0.0f, 0.0f, 1.0f);
						glBegin(GL_LINES);
						glVertex2f(textX + quadsBounds.right + boundsWidth * 0.25f, textY + baseline + scaledDescent);
						glVertex2f(textX + quadsBounds.left - boundsWidth * 0.25f, textY + baseline + scaledDescent);
						glEnd();
						glLineWidth(1.0f);

						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINE_LOOP);
						glVertex2f(textX + quadsBounds.right, textY + quadsBounds.top);
						glVertex2f(textX + quadsBounds.left, textY + quadsBounds.top);
						glVertex2f(textX + quadsBounds.left, textY + quadsBounds.bottom);
						glVertex2f(textX + quadsBounds.right, textY + quadsBounds.bottom);
						glEnd();

						for (uint32_t quadIndex = 0; quadIndex < quadCount; ++quadIndex) {
							fntFontQuad *fontQuad = fontQuads + quadIndex;

							float u0 = fontQuad->uv[0].u;
							float v0 = fontQuad->uv[0].v;
							float u1 = fontQuad->uv[1].u;
							float v1 = fontQuad->uv[1].v;

							float x0 = textX + fontQuad->coords[0].x;
							float y0 = textY + fontQuad->coords[0].y;
							float x1 = textX + fontQuad->coords[1].x;
							float y1 = textY + fontQuad->coords[1].y;

							GLuint textureId = fontTextures[fontQuad->bitmapIndex];
							glEnable(GL_TEXTURE_2D);
							glBindTexture(GL_TEXTURE_2D, textureId);
							glColor3f(1.0f, 1.0f, 1.0f);
							glBegin(GL_QUADS);
							glTexCoord2f(u1, v1); glVertex2f(x1, y1);
							glTexCoord2f(u0, v1); glVertex2f(x0, y1);
							glTexCoord2f(u0, v0); glVertex2f(x0, y0);
							glTexCoord2f(u1, v0); glVertex2f(x1, y0);
							glEnd();
							glBindTexture(GL_TEXTURE_2D, 0);
							glDisable(GL_TEXTURE_2D);
						}

						for (uint32_t quadIndex = 0; quadIndex < quadCount; ++quadIndex) {
							fntFontQuad *fontQuad = fontQuads + quadIndex;

							float x0 = textX + fontQuad->coords[0].x;
							float y0 = textY + fontQuad->coords[0].y;
							float x1 = textX + fontQuad->coords[1].x;
							float y1 = textY + fontQuad->coords[1].y;

							glColor3f(1.0f, 0.0f, 1.0f);
							glBegin(GL_LINE_LOOP);
							glVertex2f(x1, y1);
							glVertex2f(x0, y1);
							glVertex2f(x0, y0);
							glVertex2f(x1, y0);
							glEnd();
						}
					}

#if 0
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, fontTextures[1]);
					glColor3f(1.0f, 1.0f, 1.0f);
					glBegin(GL_QUADS);
					glTexCoord2f(1.0f, 0.0f); glVertex2f(w, h);
					glTexCoord2f(0.0f, 0.0f); glVertex2f(-w, h);
					glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -h);
					glTexCoord2f(1.0f, 1.0f); glVertex2f(w, -h);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
#endif

					fplVideoFlip();
				}

				for (uint32_t bitmapIndex = 0; bitmapIndex < atlas.bitmapCount; ++bitmapIndex) {
					glDeleteTextures(1, &fontTextures[bitmapIndex]);
				}

				fntFreeFontAtlas(&atlas);
			}
			fglUnloadOpenGL();
		}
		fplPlatformRelease();
	}
	return (0);
}