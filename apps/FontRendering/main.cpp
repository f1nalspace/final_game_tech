#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include <GL/GL.h>

struct BakedCodePoint {
	float s0;
	float t0;
	float s1;
	float t1;
	float w;
	float h;
	float xoffset;
	float yoffset;
	float xadvance;
	int codePoint;
};

struct TextPos {
	float x;
	float y;
};

struct TextBounds {
	float left;
	float top;
	float right;
	float bottom;
};

int main(int argc, char *argv[]) {
	fplSettings settings = {};
	fplSetDefaultSettings(&settings);
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D);

		constexpr int AtlasWidth = 2048;
		constexpr int AtlasHeight = 2048;
		constexpr int CharFirst = 32;
		constexpr int CharLast = 255;
		constexpr float FontHeight = 128.0f;
		constexpr float PixelToUnits = 1.0f / FontHeight;
		constexpr int CharCount = (CharLast - CharFirst) + 1;
		constexpr float ipw = 1.0f / AtlasWidth, iph = 1.0f / AtlasHeight;

		BakedCodePoint *bakedCodePoints = (BakedCodePoint *)fplMemoryAllocate(CharCount * sizeof(BakedCodePoint));
		float *kerningTable = (float *)fplMemoryAllocate(CharCount * CharCount * sizeof(float));

		float ascent = 0.0f;
		float descent = 0.0f;
		float lineGap = 0.0f;

		GLuint ftex = 0;

		fplFileHandle fontFile;
		if (fplFileOpenBinary("c:/windows/fonts/arial.ttf", &fontFile)) {
			uint32_t fileSize = fplFileGetSizeFromHandle32(&fontFile);
			uint8_t *ttf_buffer = (uint8_t *)fplMemoryAllocate(fileSize);
			fplFileReadBlock32(&fontFile, fileSize, ttf_buffer, fileSize);
			fplFileClose(&fontFile);

			int fontOffset = stbtt_GetFontOffsetForIndex(ttf_buffer, 0);

			stbtt_fontinfo fontInfo;
			stbtt_InitFont(&fontInfo, ttf_buffer, fontOffset);

			uint8_t *temp_bitmap = (uint8_t *)fplMemoryAllocate(AtlasWidth * AtlasHeight);

			stbtt_packedchar *packedChars = (stbtt_packedchar *)fplMemoryAllocate(CharCount * sizeof(stbtt_bakedchar));

			stbtt_pack_context packCtx;

			stbtt_pack_range packRng = {};
			packRng.font_size = FontHeight;
			packRng.first_unicode_codepoint_in_range = CharFirst;
			packRng.num_chars = CharCount;
			packRng.chardata_for_range = packedChars;

			stbtt_PackBegin(&packCtx, temp_bitmap, AtlasWidth, AtlasHeight, 0, 1, fpl_null);
			stbtt_PackFontRanges(&packCtx, ttf_buffer, 0, &packRng, 1);
			stbtt_PackEnd(&packCtx);

			//stbtt_BakeFontBitmap(ttf_buffer, 0, FontHeight, temp_bitmap, AtlasWidth, AtlasHeight, CharFirst, CharCount, bakedChars);

			for (int charIndex = 0; charIndex < CharCount; ++charIndex) {
				const stbtt_packedchar *b = packedChars + charIndex;
				BakedCodePoint *cp = bakedCodePoints + charIndex;
				cp->codePoint = CharFirst + charIndex;
				cp->w = (b->x1 - b->x0) * PixelToUnits;
				cp->h = (b->y1 - b->y0) * PixelToUnits;
				cp->xoffset = b->xoff * PixelToUnits;
				cp->yoffset = b->yoff * PixelToUnits;
				cp->xadvance = b->xadvance * PixelToUnits;
				cp->s0 = b->x0 * ipw;
				cp->t0 = b->y0 * iph;
				cp->s1 = b->x1 * ipw;
				cp->t1 = b->y1 * iph;
			}

			float rawToPixels = stbtt_ScaleForPixelHeight(&fontInfo, FontHeight);

			int ascentRaw, descentRaw, lineGapRaw;
			stbtt_GetFontVMetrics(&fontInfo, &ascentRaw, &descentRaw, &lineGapRaw);

			ascent = ascentRaw * rawToPixels * PixelToUnits;
			descent = descentRaw * rawToPixels * PixelToUnits;
			lineGap = lineGapRaw * rawToPixels * PixelToUnits;

			for (int charIndexA = 0; charIndexA < CharCount; ++charIndexA) {
				for (int charIndexB = 0; charIndexB < CharCount; ++charIndexB) {
					if (charIndexA == charIndexB) continue;

					int codePointA = CharFirst + charIndexA;
					int codePointB = CharFirst + charIndexB;

					int kerningRaw = stbtt_GetCodepointKernAdvance(&fontInfo, codePointA, codePointB);

					float kerning = (float)kerningRaw * rawToPixels * PixelToUnits;

					kerningTable[charIndexA * CharCount + charIndexB] = kerning;
				}
			}

			glGenTextures(1, &ftex);
			glBindTexture(GL_TEXTURE_2D, ftex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, AtlasWidth, AtlasHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			fplMemoryFree(packedChars);
			fplMemoryFree(temp_bitmap);
		}

		bool topDown = false;
		bool drawBounds = false;

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
									} else if (ev.keyboard.mappedKey == fplKey_B) {
										drawBounds = !drawBounds;
									}
								}
							} break;
						}
					} break;
				}
			}

			fplWindowSize winSize;
			if (!fplGetWindowSize(&winSize)) {
				winSize = { 0,0 };
			}

			float w = (float)winSize.width;
			float h = (float)winSize.height;

			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0, 0, winSize.width, winSize.height);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			if (topDown) {
				glOrtho(0.0f, w, h, 0, 0.0f, 1.0f);
			} else {
				glOrtho(0.0f, w, 0, h, 0.0f, 1.0f);
			}

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			float lw = fplMin(w, h) * 1.0f;
			glColor4f(1, 1, 0, 0.25f);
			glLineWidth(1);
			glBegin(GL_LINES);
			glVertex2f(w * 0.5f - lw * 0.5f, h * 0.5f);
			glVertex2f(w * 0.5f + lw * 0.5f, h * 0.5f);
			glVertex2f(w * 0.5f, h * 0.5f - lw * 0.5f);
			glVertex2f(w * 0.5f, h * 0.5f + lw * 0.5f);
			glEnd();
			glLineWidth(1);

			float fontScale = 128.0f;

			//const char *text = "öÖ^Final-Platform-Layer";
			//const char *text = "Bitte!";
			const char *text = "Five Wax Quacking Zephyrs";
			size_t textLen = strlen(text);

			TextPos normalPos = { w * 0.1f, h * 0.5f };
			TextPos kernedPos = { w * 0.1f, h * 0.375f };
			TextBounds normalBounds = { normalPos.x, normalPos.y, normalPos.x, normalPos.y };
			TextBounds kernedBounds = { kernedPos.x, kernedPos.y, kernedPos.x, kernedPos.y };
			for (int textIndex = 0; textIndex < textLen; ++textIndex) {
				int codePoint = (int)text[textIndex];
				if (codePoint >= CharFirst && codePoint <= CharLast) {
					int codePointIndex = codePoint - CharFirst;
					stbtt_aligned_quad quad = {};

					const BakedCodePoint *b = bakedCodePoints + codePointIndex;

					float quadWidth = b->w * fontScale;
					float quadHeight = b->h * fontScale;

					quad.s0 = b->s0;
					quad.t0 = b->t0;
					quad.s1 = b->s1;
					quad.t1 = b->t1;
					quad.x0 = b->xoffset * fontScale;
					quad.x1 = quad.x0 + b->w * fontScale;
					assert(quadWidth == (quad.x1 - quad.x0));

					if (topDown) {
						quad.y0 = b->yoffset * fontScale;
						quad.y1 = quad.y0 + b->h * fontScale;
						assert(quadHeight == (quad.y1 - quad.y0));
					} else {
						quad.y0 = -b->yoffset * fontScale;
						quad.y1 = quad.y0 - b->h * fontScale;
						assert(quadHeight == (quad.y0 - quad.y1));
					}

					float normalLeft = normalPos.x + quad.x0;
					float normalRight = normalPos.x + quad.x1;
					float normalTop = normalPos.y + quad.y1;
					float normalBottom = normalPos.y + quad.y0;

					float kernedLeft = kernedPos.x + quad.x0;
					float kernedRight = kernedPos.x + quad.x1;
					float kernedTop = kernedPos.y + quad.y1;
					float kernedBottom = kernedPos.y + quad.y0;

					float advance = b->xadvance;

					// Normal
					glBindTexture(GL_TEXTURE_2D, ftex);
					glColor4f(1, 1, 1, 1);
					glBegin(GL_QUADS);
					glTexCoord2f(quad.s1, quad.t1); glVertex2f(normalRight, normalTop);
					glTexCoord2f(quad.s0, quad.t1); glVertex2f(normalLeft, normalTop);
					glTexCoord2f(quad.s0, quad.t0); glVertex2f(normalLeft, normalBottom);
					glTexCoord2f(quad.s1, quad.t0); glVertex2f(normalRight, normalBottom);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);

					if (drawBounds) {
						glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
						glLineWidth(1.0f);
						glBegin(GL_LINE_LOOP);
						glVertex2f(normalRight, normalTop);
						glVertex2f(normalLeft, normalTop);
						glVertex2f(normalLeft, normalPos.y + quad.y0);
						glVertex2f(normalRight, normalBottom);
						glEnd();
						glLineWidth(1.0f);
					}

					// Kerned
					glBindTexture(GL_TEXTURE_2D, ftex);
					glColor4f(1, 1, 1, 1);
					glBegin(GL_QUADS);
					glTexCoord2f(quad.s1, quad.t1); glVertex2f(kernedRight, kernedTop);
					glTexCoord2f(quad.s0, quad.t1); glVertex2f(kernedLeft, kernedTop);
					glTexCoord2f(quad.s0, quad.t0); glVertex2f(kernedLeft, kernedBottom);
					glTexCoord2f(quad.s1, quad.t0); glVertex2f(kernedRight, kernedBottom);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);

					if (drawBounds) {
						glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
						glLineWidth(1.0f);
						glBegin(GL_LINE_LOOP);
						glVertex2f(kernedRight, kernedTop);
						glVertex2f(kernedLeft, kernedTop);
						glVertex2f(kernedLeft, kernedBottom);
						glVertex2f(kernedRight, kernedBottom);
						glEnd();
						glLineWidth(1.0f);
					}

					float kerning = 0.0f;
					if (textIndex < textLen - 1) {
						int nextCodePoint = (int)text[textIndex + 1];
						if (nextCodePoint >= CharFirst && nextCodePoint <= CharLast) {
							int kerningIndexA = codePointIndex;
							int kerningIndexB = nextCodePoint - CharFirst;
							kerning = kerningTable[kerningIndexA * CharCount + kerningIndexB];
						}
					}

					normalPos.x += advance * fontScale;
					kernedPos.x += (advance + kerning) * fontScale;

					normalBounds.left = fplMin(normalBounds.left, normalLeft);
					normalBounds.right = fplMax(normalBounds.right, normalRight);
					normalBounds.top = fplMax(normalBounds.top, fplMax(normalTop, normalBottom));
					normalBounds.bottom = fplMin(normalBounds.bottom, fplMin(normalTop, normalBottom));

					kernedBounds.left = fplMin(kernedBounds.left, kernedLeft);
					kernedBounds.right = fplMax(kernedBounds.right, kernedRight);
					kernedBounds.top = fplMax(kernedBounds.top, fplMax(kernedTop, kernedBottom));
					kernedBounds.bottom = fplMin(kernedBounds.bottom, fplMin(kernedTop, kernedBottom));
				}
			}

			if (drawBounds) {
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				glLineWidth(1.0f);
				glBegin(GL_LINE_LOOP);
				glVertex2f(normalBounds.right, normalBounds.top);
				glVertex2f(normalBounds.left, normalBounds.top);
				glVertex2f(normalBounds.left, normalBounds.bottom);
				glVertex2f(normalBounds.right, normalBounds.bottom);
				glEnd();
				glLineWidth(1.0f);

				glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
				glLineWidth(1.0f);
				glBegin(GL_LINE_LOOP);
				glVertex2f(kernedBounds.right, kernedBounds.top);
				glVertex2f(kernedBounds.left, kernedBounds.top);
				glVertex2f(kernedBounds.left, kernedBounds.bottom);
				glVertex2f(kernedBounds.right, kernedBounds.bottom);
				glEnd();
				glLineWidth(1.0f);
			}

			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}