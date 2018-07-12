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
	char c;
};

int main(int argc, char *argv[]) {
	fplSettings settings = {};
	fplSetDefaultSettings(&settings);
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if(fplPlatformInit(fplInitFlags_Video, &settings)) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D);

		constexpr int AtlasWidth = 512;
		constexpr int AtlasHeight = 512;
		constexpr int CharFirst = 32;
		constexpr int CharLast = 128;
		constexpr float FontHeight = 36.0f;
		constexpr float PixelToUnits = 1.0f / FontHeight;
		constexpr int CharCount = (CharLast - CharFirst) + 1;
		stbtt_bakedchar *bakedChars = (stbtt_bakedchar *)fplMemoryAllocate(CharCount * sizeof(stbtt_bakedchar));
		BakedCodePoint *bakedCodePoints = (BakedCodePoint *)fplMemoryAllocate(CharCount * sizeof(BakedCodePoint));

		GLuint ftex = 0;

		fplFileHandle fontFile;
		if(fplOpenAnsiBinaryFile("c:/windows/fonts/times.ttf", &fontFile)) {
			uint32_t fileSize = fplGetFileSizeFromHandle32(&fontFile);
			uint8_t *ttf_buffer = (uint8_t *)fplMemoryAllocate(fileSize);
			fplReadFileBlock32(&fontFile, fileSize, ttf_buffer, fileSize);
			fplCloseFile(&fontFile);

			uint8_t *temp_bitmap = (uint8_t *)fplMemoryAllocate(AtlasWidth * AtlasHeight);
			stbtt_BakeFontBitmap(ttf_buffer, 0, FontHeight, temp_bitmap, AtlasWidth, AtlasHeight, CharFirst, CharLast, bakedChars);

			float ipw = 1.0f / AtlasWidth, iph = 1.0f / AtlasHeight;
			for(int charIndex = 0; charIndex < CharCount; ++charIndex) {
				const stbtt_bakedchar *b = bakedChars + charIndex;
				BakedCodePoint *cp = bakedCodePoints + charIndex;
				cp->c = charIndex + CharFirst;
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

			glGenTextures(1, &ftex);
			glBindTexture(GL_TEXTURE_2D, ftex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			fplMemoryFree(temp_bitmap);
		}

		bool topDown = false;

		while(fplWindowUpdate()) {
			fplEvent ev;
			while(fplPollEvent(&ev)) {
				switch(ev.type) {
					case fplEventType_Keyboard:
					{
						switch(ev.keyboard.type) {
							case fplKeyboardEventType_Button:
							{
								if(ev.keyboard.buttonState == fplButtonState_Release) {
									if(ev.keyboard.mappedKey == fplKey_Space) {
										topDown = !topDown;
									}
								}
							} break;
						}
					} break;
				}
			}

			fplWindowSize winSize;
			if(!fplGetWindowArea(&winSize)) {
				winSize = { 0,0 };
			}

			float w = (float)winSize.width;
			float h = (float)winSize.height;

			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0, 0, winSize.width, winSize.height);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			if(topDown) {
				glOrtho(0.0f, w, h, 0, 0.0f, 1.0f);
			} else {
				glOrtho(0.0f, w, 0, h, 0.0f, 1.0f);
			}

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			float lw = FPL_MIN(w, h) * 0.25f;
			glColor4f(1, 1, 0, 0.25f);
			glLineWidth(1);
			glBegin(GL_LINES);
			glVertex2f(w * 0.5f - lw * 0.5f, h * 0.5f);
			glVertex2f(w * 0.5f + lw * 0.5f, h * 0.5f);
			glVertex2f(w * 0.5f, h * 0.5f - lw * 0.5f);
			glVertex2f(w * 0.5f, h * 0.5f + lw * 0.5f);
			glEnd();
			glLineWidth(1);

			float fontScale = 64.0f;

			const char *text = "Hello World!";

			float x = w * 0.5f;
			float y = h * 0.5f;
			while(*text) {
				if(*text >= CharFirst && *text <= CharLast) {
					char c = *text;
					int charIndex = c - CharFirst;
					stbtt_aligned_quad q0, q1;

					const stbtt_bakedchar *b0 = bakedChars + charIndex;
					float ipw = 1.0f / AtlasWidth, iph = 1.0f / AtlasHeight;
					float xoffset = b0->xoff * PixelToUnits;
					float yoffset = b0->yoff * PixelToUnits;
					float boundW = (b0->x1 - b0->x0) * PixelToUnits;
					float boundH = (b0->y1 - b0->y0) * PixelToUnits;
					float advance0 = b0->xadvance * PixelToUnits;

					q0.x0 = xoffset * fontScale;
					q0.x1 = q0.x0 + boundW * fontScale;
					if(topDown) {
						q0.y0 = yoffset * fontScale;
						q0.y1 = q0.y0 + boundH * fontScale;
					} else {
						q0.y0 = -yoffset * fontScale;
						q0.y1 = q0.y0 - boundH * fontScale;
					}
					q0.s0 = b0->x0 * ipw;
					q0.t0 = b0->y0 * iph;
					q0.s1 = b0->x1 * ipw;
					q0.t1 = b0->y1 * iph;

					const BakedCodePoint *b1 = bakedCodePoints + charIndex;
					q1.s0 = b1->s0;
					q1.t0 = b1->t0;
					q1.s1 = b1->s1;
					q1.t1 = b1->t1;
					q1.x0 = b1->xoffset * fontScale;
					q1.x1 = q1.x0 + b1->w * fontScale;
					if(topDown) {
						q1.y0 = b1->yoffset * fontScale;
						q1.y1 = q1.y0 + b1->h * fontScale;
					} else {
						q1.y0 = -b1->yoffset * fontScale;
						q1.y1 = q1.y0 - b1->h * fontScale;
					}
					float advance1 = b1->xadvance;

					assert(b1->c == c);
					assert(q0.s0 == q1.s0);
					assert(q0.s1 == q1.s1);
					assert(q0.t0 == q1.t0);
					assert(q0.t1 == q1.t1);
					assert(q0.x0 == q1.x0);
					assert(q0.x1 == q1.x1);
					assert(q0.y0 == q1.y0);
					assert(q0.y1 == q1.y1);

#if 0
					q1 = q0;
					advance1 = advance0;
#endif

					glBindTexture(GL_TEXTURE_2D, ftex);
					glColor4f(1, 1, 1, 1);
					glBegin(GL_QUADS);
					glTexCoord2f(q1.s1, q1.t1); glVertex2f(x + q1.x1, y + q1.y1);
					glTexCoord2f(q1.s0, q1.t1); glVertex2f(x + q1.x0, y + q1.y1);
					glTexCoord2f(q1.s0, q1.t0); glVertex2f(x + q1.x0, y + q1.y0);
					glTexCoord2f(q1.s1, q1.t0); glVertex2f(x + q1.x1, y + q1.y0);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);

					glColor4f(0, 1.0f, 0.0f, 1.0f);
					glLineWidth(1.0f);
					glBegin(GL_LINE_LOOP);
					glVertex2f(x + q1.x1, y + q1.y1);
					glVertex2f(x + q1.x0, y + q1.y1);
					glVertex2f(x + q1.x0, y + q1.y0);
					glVertex2f(x + q1.x1, y + q1.y0);
					glEnd();
					glLineWidth(1.0f);

					x += advance1 * fontScale;
				}
				++text;
			}

			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}