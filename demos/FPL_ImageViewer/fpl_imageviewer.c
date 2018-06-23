/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImageViewer
Description:
	Very simple opengl based image viewer
Requirements:
	- C99 Compiler
	- Final Dynamic OpenGL
	- STBimage
Author:
	Torsten Spaete
Changelog:
	## 2018-06-23
	- Initial version
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string.h>
#include <assert.h>

#define MAX_PICTURE_FILE_COUNT 1024
typedef struct PictureFile {
	char filename[FPL_MAX_FILENAME_LENGTH];
} PictureFile;

typedef enum PictureFormatType {
	PictureFormatType_None = 0,
	PictureFormatType_RGB8,
	PictureFormatType_RGBA8,
} PictureFormatType;

typedef struct ActivePicture {
	char filePath[FPL_MAX_PATH_LENGTH];
	size_t index;
	uint32_t width;
	uint32_t height;
	GLuint textureId;
	bool isLoaded;
} ActivePicture;

typedef struct ViewerState {
	char picturesPath[FPL_MAX_PATH_LENGTH];
	PictureFile pictureFiles[MAX_PICTURE_FILE_COUNT];
	ActivePicture activePicture;
	size_t pictureFileCount;
} ViewerState;

static bool IsImageFile(const char *filePath) {
	const char *ext = fplExtractFileExtension(filePath);
	bool result = (_stricmp(ext, ".jpg") == 0) || (_stricmp(ext, ".jpeg") == 0) || (_stricmp(ext, ".png") == 0) || (_stricmp(ext, ".bmp") == 0);
	return(result);
}

static void PushPictureFile(ViewerState *state, const fplFileEntry *entry) {
	assert(state->pictureFileCount < FPL_ARRAYCOUNT(state->pictureFiles));
	PictureFile *pictureFile = &state->pictureFiles[state->pictureFileCount++];
	const char *filename = fplExtractFileName(entry->fullPath);
	fplCopyAnsiString(filename, pictureFile->filename, FPL_ARRAYCOUNT(pictureFile->filename));
}

static bool LoadPicturesPath(ViewerState *state, const char *path) {
	state->pictureFileCount = 0;
	state->picturesPath[0] = 0;
	if (fplDirectoryExists(path)) {
		fplCopyAnsiString(path, state->picturesPath, FPL_ARRAYCOUNT(state->picturesPath));
		fplFileEntry entry;
		if (fplListDirBegin(path, "*", &entry)) {
			if (IsImageFile(entry.fullPath)) {
				PushPictureFile(state, &entry);
			}
			while (fplListDirNext(&entry)) {
				if (IsImageFile(entry.fullPath)) {
					PushPictureFile(state, &entry);
				}
			}
			fplListDirEnd(&entry);
		}
	} else if (fplFileExists(path)) {
		if (IsImageFile(path)) {
			fplExtractFilePath(path, state->picturesPath, FPL_ARRAYCOUNT(state->picturesPath));
			state->pictureFileCount = 1;
			const char *filename = fplExtractFileName(path);
			fplCopyAnsiString(filename, state->pictureFiles[0].filename, FPL_ARRAYCOUNT(state->pictureFiles[0].filename));
		}
	}
	bool result = (state->pictureFileCount > 0);
	return(result);
}

static void ReleaseTexture(GLuint *target) {
	assert(*target > 0);
	glDeleteTextures(1, target);
	*target = 0;
}

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const uint8_t components, const void *data, const bool repeatable, const GLint filter) {
	static int internalFormatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA8,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB8,
		/* 4 = RGBA */ GL_RGBA8,
	};
	static int formatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB,
		/* 4 = RGBA */ GL_RGBA,
	};
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_RECTANGLE, handle);
	GLuint internalFormat = internalFormatMapping[components];
	GLenum format = formatMapping[components];
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	return(handle);
}

static void LoadPicture(ViewerState *state, const size_t index) {
	fplDebugFormatOut("-------------------\n");

	double s, e;

	assert(state->picturesPath != fpl_null);
	assert(index >= 0 && index < state->pictureFileCount);
	if (state->activePicture.isLoaded) {
		s = fplGetTimeInMillisecondsHP();
		ReleaseTexture(&state->activePicture.textureId);
		state->activePicture.filePath[0] = 0;
		state->activePicture.isLoaded = false;
		e = fplGetTimeInMillisecondsHP();
		fplDebugFormatOut("Release: %f ms\n", (e - s));
	}
	PictureFile *picFile = &state->pictureFiles[index];
	assert(picFile->filename != fpl_null);
	fplPathCombine(state->activePicture.filePath, FPL_ARRAYCOUNT(state->activePicture.filePath), 2, state->picturesPath, picFile->filename);

	int w, h, comp;
	s = fplGetTimeInMillisecondsHP();
	uint8_t *data = stbi_load(state->activePicture.filePath, &w, &h, &comp, 0);
	e = fplGetTimeInMillisecondsHP();
	fplDebugFormatOut("Load: %f ms\n", (e - s));
	if (data != fpl_null) {
		assert(w > 0 && h > 0 && comp > 0);
		s = fplGetTimeInMillisecondsHP();
		state->activePicture.textureId = AllocateTexture(w, h, comp, data, false, GL_LINEAR);
		e = fplGetTimeInMillisecondsHP();
		state->activePicture.width = w;
		state->activePicture.height = h;
		state->activePicture.index = index;
		state->activePicture.isLoaded = true;
		fplDebugFormatOut("Upload: %f ms\n", (e - s));
		s = fplGetTimeInMillisecondsHP();
		stbi_image_free(data);
		e = fplGetTimeInMillisecondsHP();
		fplDebugFormatOut("Free: %f\n", (e - s));
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		return -1;
	}

	int returnCode = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	settings.video.driver = fplVideoDriverType_OpenGL;
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		if (fglLoadOpenGL(true)) {

			glClearColor(0, 0, 0, 1);
			glEnable(GL_TEXTURE_RECTANGLE);

			ViewerState state = FPL_ZERO_INIT;
			if (LoadPicturesPath(&state, argv[1])) {
				LoadPicture(&state, 0);
			}

			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					switch (ev.type) {
						case fplEventType_Keyboard:
						{
							if (ev.keyboard.type == fplKeyboardEventType_KeyUp) {
								if (ev.keyboard.mappedKey == fplKey_Left) {
									if (state.activePicture.index > 0) {
										LoadPicture(&state, state.activePicture.index - 1);
									}
								} else if (ev.keyboard.mappedKey == fplKey_Right) {
									if (state.activePicture.index < (state.pictureFileCount - 1)) {
										LoadPicture(&state, state.activePicture.index + 1);
									}
								}
							}
						} break;
					}
				}

				int w, h;
				fplWindowSize winSize;
				if (fplGetWindowArea(&winSize)) {
					w = winSize.width;
					h = winSize.height;
				} else {
					w = 0;
					h = 0;
				}

				float screenLeft = -(float)w * 0.5f;
				float screenRight = (float)w * 0.5f;
				float screenBottom = -(float)h * 0.5f;
				float screenTop = (float)h * 0.5f;
				float screenW = (float)w;
				float screenH = (float)h;

				glClear(GL_COLOR_BUFFER_BIT);

				glViewport(0, 0, w, h);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(screenLeft, screenRight, screenBottom, screenTop, 0.0f, 1.0f);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				float texW = (float)state.activePicture.width;
				float texH = (float)state.activePicture.height;
				float aspect = texH > 0 ? texW / texH : 0;
				assert(aspect != 0);

				float targetHeight = screenW / aspect;
				float viewWidth = screenW;
				float viewHeight = screenH;
				float viewX;
				float viewY;
				if (targetHeight > screenH) {
					viewHeight = screenH;
					viewWidth = screenH * aspect;
					viewX = screenLeft + (screenW - viewWidth) * 0.5f;
					viewY = screenBottom;
				} else {
					viewWidth = screenW;
					viewHeight = screenW / aspect;
					viewX = screenLeft;
					viewY = screenBottom + (screenH - viewHeight) * 0.5f;
				}
				float viewLeft = viewX;
				float viewRight = viewX + viewWidth;
				float viewBottom = viewY;
				float viewTop = viewY + viewHeight;

				glBindTexture(GL_TEXTURE_RECTANGLE, state.activePicture.textureId);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_QUADS);
				glTexCoord2f(texW, 0.0f); glVertex2f(viewRight, viewTop);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(viewLeft, viewTop);
				glTexCoord2f(0.0f, texH); glVertex2f(viewLeft, viewBottom);
				glTexCoord2f(texW, texH); glVertex2f(viewRight, viewBottom);
				glEnd();
				glBindTexture(GL_TEXTURE_RECTANGLE, 0);

				fplVideoFlip();
			}
			fglUnloadOpenGL();
		} else {
			returnCode = -1;
		}
		fplPlatformRelease();
	} else {
		returnCode = -1;
	}
	return(returnCode);
}