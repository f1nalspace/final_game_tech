#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

int main(int argc, char **args) {
	using namespace fpl;
	using namespace fpl::memory;
	using namespace fpl::window;

	int result = 0;
	if (InitPlatform(InitFlags::VideoOpenGL)) {
		SetWindowArea(640, 480);
		SetWindowPosition(0, 0);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.39f, 0.58f, 0.93f, 1.0f);

		uint32_t textureWidth = 128;
		uint32_t textureHeight = 128;
		uint8_t *textureData = (uint8_t *)AllocateMemory(textureWidth * textureHeight * 4);
		uint32_t *pixelPtr = (uint32_t *)textureData;
		for (uint32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; ++pixelIndex) {
			uint8_t r = 255;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 255;
			pixelPtr[pixelIndex] = (a << 24) | (b << 16) | (g << 8) | (r << 0);
		}

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		glBindTexture(GL_TEXTURE_2D, 0);

		FreeMemory(textureData);

		float rot = 0.0f;
		while (WindowUpdate()) {
			WindowSize windowArea = GetWindowArea();

			glViewport(0, 0, windowArea.width, windowArea.height);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, textureId);
			glColor3f(1.0f, 1.0f, 1.0f);

			glPushMatrix();
			glRotatef(rot, 0, 0, 1);
			glBegin(GL_QUADS);
			glVertex2f(0.5f, 0.5f);
			glVertex2f(-0.5f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();
			glPopMatrix();

			glBindTexture(GL_TEXTURE_2D, 0);

			WindowFlip();

			rot += 0.5f;
		}

		glDeleteTextures(1, &textureId);

		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}