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

			//fontData = LoadFontFromFile("c:/windows/fonts/arial.ttf");
#if 1
			fontData.size = fontAvrilSansRegularLength;
			fontData.data = fontAvrilSansRegularPtr;
			fontData.name = fontAvrilSansRegularName;
			fontData.index = 0;
#endif

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

				fntFontQuad fontQuads[64];
				fntBounds quadsBounds;
				size_t quadCount;

#if 0
				bool r;

				quadCount = fntGetQuadCountFromUTF8(helloWorldText);
				fplAssert(quadCount == 12);
				r = fntComputeQuadsFromUTF8(&atlas, helloWorldText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds);
				fplAssert(r == true);

				quadCount = fntGetQuadCountFromUTF8(testText);
				fplAssert(quadCount == 25);
				r = fntComputeQuadsFromUTF8(&atlas, testText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds);
				fplAssert(r == true);

				quadCount = fntGetQuadCountFromUTF8(japAnimeText);
				fplAssert(quadCount == 3);
				r = fntComputeQuadsFromUTF8(&atlas, japAnimeText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds);
				fplAssert(r == true);

				quadCount = fntGetQuadCountFromUTF8(japAnimeAndKanaText);
				fplAssert(quadCount == 9);
				r = fntComputeQuadsFromUTF8(&atlas, japAnimeAndKanaText, targetCharHeight, fntComputeQuadsFlags_None, fplArrayCount(fontQuads), fontQuads, &quadsBounds);
				fplAssert(r == true);
#endif

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

				GLuint fontTextures[8] = fplZeroInit;

				for (uint32_t bitmapIndex = 0; bitmapIndex < atlas.bitmapCount; ++bitmapIndex) {
					fontTextures[bitmapIndex] = CreateRGBATextureFromAlpha(atlas.bitmaps[bitmapIndex].pixels, atlas.bitmaps[bitmapIndex].width, atlas.bitmaps[bitmapIndex].height);
				}

				glClearColor(0.7f, 0.3f, 0.1f, 1.0f);
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

					//const char *text = "The quick brown fox jumps over the lazy dog";
					const char *text = "Five Wax Quacking Zephyrs";

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

					if (fntComputeQuadsFromUTF8(&atlas, text, fontSize.f32, flags, fplArrayCount(fontQuads), fontQuads, &quadsBounds)) {
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