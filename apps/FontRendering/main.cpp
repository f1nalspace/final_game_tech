// FontRendering — exercises final_fontloader.h on a real TTF, draws kerned vs unkerned text and bounds.
// Topdown vs cartesian Y is toggled with Space, bounds visualization with B.

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#include <final_fontloader.h>

#include <final_fonts.h>

#include <GL/gl.h>

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
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	if (!fplPlatformInit(fplInitFlags_Video, &settings)) {
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	const uint32_t AtlasWidth = 2048;
	const uint32_t AtlasHeight = 2048;
	const uint32_t CharFirst = 32;
	const uint32_t CharLast = 255;
	const float FontHeight = 128.0f;

	LoadedFont font = {};
	GLuint ftex = 0;
	if (FontLoadFromMemoryEx(fpl_null, ptr_fontSulphurPointRegular, sizeOf_fontSulphurPointRegular, 0, FontHeight, CharFirst, CharLast, AtlasWidth, AtlasHeight, true, FontQuality_Packed1x, &font)) {
		glGenTextures(1, &ftex);
		glBindTexture(GL_TEXTURE_2D, ftex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font.atlasWidth, font.atlasHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font.atlasAlphaBitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool topDown = false;
	bool drawBounds = false;

	while (fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			if (ev.type == fplEventType_Keyboard && ev.keyboard.type == fplKeyboardEventType_Button && ev.keyboard.buttonState == fplButtonState_Release) {
				if (ev.keyboard.mappedKey == fplKey_Space) {
					topDown = !topDown;
				} else if (ev.keyboard.mappedKey == fplKey_B) {
					drawBounds = !drawBounds;
				}
			}
		}

		fplWindowSize winSize;
		if (!fplGetWindowSize(&winSize)) {
			winSize.width = 0;
			winSize.height = 0;
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

		float lw = fplMin(w, h);
		glColor4f(1, 1, 0, 0.25f);
		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex2f(w * 0.5f - lw * 0.5f, h * 0.5f);
		glVertex2f(w * 0.5f + lw * 0.5f, h * 0.5f);
		glVertex2f(w * 0.5f, h * 0.5f - lw * 0.5f);
		glVertex2f(w * 0.5f, h * 0.5f + lw * 0.5f);
		glEnd();

		if (ftex != 0) {
			const char *text = "Five Wax Quacking Zephyrs";
			const float fontScale = 128.0f;
			float ySign = topDown ? 1.0f : -1.0f;

			float normalX = w * 0.1f, normalY = h * 0.5f;
			float kernedX = w * 0.1f, kernedY = h * 0.375f;
			TextBounds normalBounds = { normalX, normalY, normalX, normalY };
			TextBounds kernedBounds = { kernedX, kernedY, kernedX, kernedY };

			size_t textLen = strlen(text);
			for (size_t i = 0; i < textLen; ++i) {
				uint32_t codePoint = (uint8_t)text[i];
				if (codePoint < CharFirst || codePoint > CharLast) {
					continue;
				}
				uint32_t nextCodePoint = (i + 1 < textLen) ? (uint8_t)text[i + 1] : 0;

				FontQuad quad = FontGetQuad(&font, codePoint, fontScale);
				// FontGetQuad's offset is to the glyph center; convert to caller-anchored corners.
				float qx0 = quad.offset.x - quad.size.x * 0.5f;
				float qy0 = quad.offset.y - quad.size.y * 0.5f;
				float qx1 = qx0 + quad.size.x;
				float qy1 = qy0 + quad.size.y;

				float u0 = quad.uvMin.x;
				float u1 = quad.uvMax.x;
				float v0 = quad.uvMin.y;
				float v1 = quad.uvMax.y;

				float advanceOnly = FontGetCharacterAdvance(&font, codePoint, 0) * fontScale;
				float n_l = normalX + qx0;
				float n_r = normalX + qx1;
				float n_t = normalY + ySign * qy1;
				float n_b = normalY + ySign * qy0;

				glBindTexture(GL_TEXTURE_2D, ftex);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_QUADS);
				glTexCoord2f(u1, v0); glVertex2f(n_r, n_t);
				glTexCoord2f(u0, v0); glVertex2f(n_l, n_t);
				glTexCoord2f(u0, v1); glVertex2f(n_l, n_b);
				glTexCoord2f(u1, v1); glVertex2f(n_r, n_b);
				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);

				if (drawBounds) {
					glColor4f(1, 0, 0, 1);
					glBegin(GL_LINE_LOOP);
					glVertex2f(n_r, n_t);
					glVertex2f(n_l, n_t);
					glVertex2f(n_l, n_b);
					glVertex2f(n_r, n_b);
					glEnd();
				}

				float advanceKerned = FontGetCharacterAdvance(&font, codePoint, nextCodePoint) * fontScale;
				float k_l = kernedX + qx0;
				float k_r = kernedX + qx1;
				float k_t = kernedY + ySign * qy1;
				float k_b = kernedY + ySign * qy0;

				glBindTexture(GL_TEXTURE_2D, ftex);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_QUADS);
				glTexCoord2f(u1, v0); glVertex2f(k_r, k_t);
				glTexCoord2f(u0, v0); glVertex2f(k_l, k_t);
				glTexCoord2f(u0, v1); glVertex2f(k_l, k_b);
				glTexCoord2f(u1, v1); glVertex2f(k_r, k_b);
				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);

				if (drawBounds) {
					glColor4f(0, 1, 0, 1);
					glBegin(GL_LINE_LOOP);
					glVertex2f(k_r, k_t);
					glVertex2f(k_l, k_t);
					glVertex2f(k_l, k_b);
					glVertex2f(k_r, k_b);
					glEnd();
				}

				normalBounds.left = fplMin(normalBounds.left, n_l);
				normalBounds.right = fplMax(normalBounds.right, n_r);
				normalBounds.top = fplMax(normalBounds.top, fplMax(n_t, n_b));
				normalBounds.bottom = fplMin(normalBounds.bottom, fplMin(n_t, n_b));

				kernedBounds.left = fplMin(kernedBounds.left, k_l);
				kernedBounds.right = fplMax(kernedBounds.right, k_r);
				kernedBounds.top = fplMax(kernedBounds.top, fplMax(k_t, k_b));
				kernedBounds.bottom = fplMin(kernedBounds.bottom, fplMin(k_t, k_b));

				normalX += advanceOnly;
				kernedX += advanceKerned;
			}

			if (drawBounds) {
				glColor4f(0, 0, 1, 1);
				glBegin(GL_LINE_LOOP);
				glVertex2f(normalBounds.right, normalBounds.top);
				glVertex2f(normalBounds.left, normalBounds.top);
				glVertex2f(normalBounds.left, normalBounds.bottom);
				glVertex2f(normalBounds.right, normalBounds.bottom);
				glEnd();

				glColor4f(0, 1, 1, 1);
				glBegin(GL_LINE_LOOP);
				glVertex2f(kernedBounds.right, kernedBounds.top);
				glVertex2f(kernedBounds.left, kernedBounds.top);
				glVertex2f(kernedBounds.left, kernedBounds.bottom);
				glVertex2f(kernedBounds.right, kernedBounds.bottom);
				glEnd();
			}
		}

		fplVideoFlip();
	}

	if (ftex != 0) {
		glDeleteTextures(1, &ftex);
	}
	FontFree(fpl_null, &font);
	fplPlatformRelease();
	return 0;
}
