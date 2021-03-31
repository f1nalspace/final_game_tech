/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | NBodySimulation

Description:
	Multi-Threaded N-Body 2D Smoothed Particle Hydrodynamics Fluid Simulation based on paper "Particle-based Viscoelastic Fluid Simulation" by Simon Clavet, Philippe Beaudoin, and Pierre Poulin.
	A experiment about creating a two-way particle simulation in 4 different programming styles to see the difference in performance and maintainability. The core math is same for all implementations, including rendering and threading.
	The core math is same for all implementations, including rendering and threading.

	Demos:
		1. Object oriented style 1 (Naive)
		2. Object oriented style 2 (Public, reserved vectors, fixed grid, no unneccesary classes or pointers)
		3. Object oriented style 3 (Structs only, no virtual function calls, reserved vectors, fixed grid)
		4. Data oriented style with 8/16 byte aligned structures

	Benchmark:
		There is a benchmark recording and rendering built-in.

		To start a benchmark hit "B" key.
		To stop a benchmark hit "Escape" key.

	Notes:
		Collision detection is discrete, therefore particles may pass through bodies when they are too thin and particles too fast.

Requirements:
	- C++/11 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Todo:
	- Migrate all GUI/Text rendering to imGUI
	- External particle forces
	- Add bar value labels on benchmark chart
	- Migrate to modern opengl 3.3+

Changelog:
	#1.4.3:
	- Migrated to FPL 0.9.2.0 beta

	#1.4.2:
	- Migrated to FPL 0.8.3.0 beta

	#1.4.1:

	- Migrated to FPL 0.8.0.0 beta
	- Migrated to FGL 0.3.2.0 beta
	- Use fplThreadYield() instead of fplThreadSleep()

	#1.4:
	- Migrated to FPL 0.7.8.0 beta
	- Replaced GLEW with final_dynamic_opengl.h
	- Replaced all std threading stuff with FPL equivalents
	- Replaced sprintf_s to fplFormatAnsiString

	#1.3:
	- Migrated to FPL 0.3.3 alpha

	#1.2:
	- Using command buffer instead of immediate rendering, so we render only in main.cpp
	- Rendering text using stb_truetype

	#1.1:
	- Improved benchmark functionality and rendering

	#1.0:
	- Added integrated benchmark functionality

	#0.9:
	- Initial version

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
*/
#define FPL_ENTRYPOINT
#define FPL_NO_AUDIO
#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <chrono>
#include <stack>

#include "app.cpp"
#include "utils.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

static Application *globalApp = nullptr;
static float lastFrameTime = 0.0f;
static uint64_t lastFrameCycles = 0;
static uint64_t lastCycles = 0;
static fplWallClock lastFrameClock;

static void OpenGLPopVertexIndexArray(std::stack<Render::VertexIndexArrayHeader *> &stack) {
	if (stack.size() > 0) {
		Render::VertexIndexArrayHeader *header = stack.top();
		stack.pop();
		if (header->colors != nullptr) {
			glDisableClientState(GL_COLOR_ARRAY);
		}
		if (header->texcoords != nullptr) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

static void OpenGLPushVertexIndexArray(std::stack<Render::VertexIndexArrayHeader *> &stack, Render::VertexIndexArrayHeader *header) {
	GLsizei vertexStride = (GLsizei)header->vertexStride;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, vertexStride, header->vertices);
	if (header->texcoords != nullptr) {
		assert(header->texcoords != header->vertices);
		GLsizei texcoordStride = (GLsizei)header->texcoordStride;
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, texcoordStride, header->texcoords);
	}
	if (header->colors != nullptr) {
		assert(header->colors != header->vertices);
		GLsizei colorStride = (GLsizei)header->colorStride;
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, colorStride, header->colors);
	}
	stack.push(header);
}

static void OpenGLAllocateTexture(Render::CommandBuffer *commandBuffer, Render::TextureOperationAllocate &allocate) {
	assert(sizeof(GLuint) <= sizeof(Render::TextureHandle *));
	assert((allocate.width > 0) && (allocate.height > 0));
	size_t textureSizeRGBA = allocate.width * allocate.height * 4;
	uint8_t *texturePixelsRGBA = PushSize<uint8_t>(&commandBuffer->textureData, textureSizeRGBA);
	// @TODO: Handle bottom-up (!isTopDown) by setting the initial source pixel and swap sign in source-increment!
	switch (allocate.bytesPerPixel) {
		case 1:
		{
			// Alpha > RGBA
			size_t sourceIncrement = 1;
			size_t sourceScanline = allocate.width;
			uint32_t *destPixel32 = (uint32_t *)texturePixelsRGBA;
			for (size_t y = 0; y < allocate.height; ++y) {
				uint8_t *sourceRow8 = (uint8_t *)allocate.data + (sourceScanline * (!allocate.isTopDown ? allocate.height - 1 - y : y));
				uint8_t *sourcePixel8 = sourceRow8;
				for (size_t x = 0; x < allocate.width; ++x) {
					uint8_t alpha = *sourcePixel8;
					Vec4f color = AlphaToLinear(alpha);
					if (!allocate.isPreMultiplied) {
						color.rgb *= color.a;
					}
					*destPixel32 = LinearToRGBA32(color);
					++destPixel32;
					sourcePixel8 += sourceIncrement;
				}
			}
		} break;
		case 3:
		{
			// RGB > RGBA
			size_t sourceIncrement = 3;
			size_t sourceScanline = allocate.width * 3;
			uint32_t *destPixel32 = (uint32_t *)texturePixelsRGBA;
			for (size_t y = 0; y < allocate.height; ++y) {
				uint8_t *sourceRow8 = (uint8_t *)allocate.data + (sourceScanline * (!allocate.isTopDown ? allocate.height - 1 - y : y));
				uint8_t *sourcePixel8 = sourceRow8;
				for (size_t x = 0; x < allocate.width; ++x) {
					uint8_t r = *(sourcePixel8 + 0);
					uint8_t g = *(sourcePixel8 + 1);
					uint8_t b = *(sourcePixel8 + 2);
					Pixel pixel = { r, g, b, 255 };
					Vec4f color = PixelToLinear(pixel);
					if (!allocate.isPreMultiplied) {
						color.rgb *= color.a;
					}
					*destPixel32 = LinearToRGBA32(color);
					++destPixel32;
					sourcePixel8 += sourceIncrement;
				}
			}
		} break;
		case 4:
		{
			// RBGA > RGBA
			size_t sourceIncrement = 4;
			size_t sourceScanline = allocate.width * 4;
			uint32_t *destPixel32 = (uint32_t *)texturePixelsRGBA;
			for (size_t y = 0; y < allocate.height; ++y) {
				uint32_t *sourceRow32 = (uint32_t *)allocate.data + (sourceScanline * (!allocate.isTopDown ? allocate.height - 1 - y : y));
				uint32_t *sourcePixel32 = sourceRow32;
				for (size_t x = 0; x < allocate.width; ++x) {
					uint32_t rgba = *sourcePixel32;
					Vec4f color = RGBA32ToLinear(rgba);
					if (!allocate.isPreMultiplied) {
						color.rgb *= color.a;
					}
					*destPixel32 = LinearToRGBA32(color);
					++destPixel32;
					sourcePixel32 += sourceIncrement;
				}
			}
		} break;
	}

	GLuint textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, allocate.width, allocate.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturePixelsRGBA);
	// @TODO: Support for multiple filters and wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	PopSize(&commandBuffer->textureData, textureSizeRGBA);

	void *result = ValueToPointer<GLuint>(textureHandle);
	*allocate.targetHandle = result;
}

static void OpenGLReleaseTexture(Render::CommandBuffer *commandBuffer, Render::TextureOperationRelease &release) {
	GLuint textureHandle = PointerToValue<GLuint>(release.handle);
	if (textureHandle > 0) {
		glDeleteTextures(1, &textureHandle);
		*release.handle = nullptr;
	}
}

inline void OpenGLCheckError() {
	GLenum error = glGetError();
	char *errorStr = nullptr;
	if (error != GL_NO_ERROR) {
		switch (error) {
			case GL_INVALID_ENUM:
				errorStr = "Invalid Enum";
				break;
			case GL_INVALID_VALUE:
				errorStr = "Invalid Value";
				break;
			case GL_INVALID_OPERATION:
				errorStr = "Invalid Operation";
				break;
			case GL_STACK_OVERFLOW:
				errorStr = "Stack Overflow";
				break;
			case GL_STACK_UNDERFLOW:
				errorStr = "Stack Underflow";
				break;
			case GL_OUT_OF_MEMORY:
				errorStr = "Out of Memory";
				break;
		}
	}
	assert(error == GL_NO_ERROR);
}

static void OpenGLDrawCommandBuffer(Render::CommandBuffer *commandBuffer) {
	// Allocate / Release textures
	commandBuffer->textureData.offset = 0;
	while (!commandBuffer->textureOperations.empty()) {
		Render::TextureOperation textureOperation = commandBuffer->textureOperations.top();
		commandBuffer->textureOperations.pop();
		switch (textureOperation.type) {
			case Render::TextureOperationType::Allocate:
			{
				OpenGLAllocateTexture(commandBuffer, textureOperation.allocate);
			} break;
			case Render::TextureOperationType::Release:
			{
				OpenGLReleaseTexture(commandBuffer, textureOperation.release);
			} break;
		}
		OpenGLCheckError();
	}

	// Render buffer
	std::stack<Render::VertexIndexArrayHeader *> vertexIndexArrayStack;
	uint8_t *commandBase = (uint8_t *)globalApp->commandBuffer->commands.base;
	uint8_t *commandAt = commandBase;
	uint8_t *commandEnd = commandAt + globalApp->commandBuffer->commands.offset;
	while (commandAt < commandEnd) {
		Render::CommandHeader *commandHeader = static_cast<Render::CommandHeader *>((void *)commandAt);
		assert(commandHeader != nullptr && commandHeader->dataSize > 0);
		commandAt += sizeof(Render::CommandHeader);

		if (!((commandHeader->type == Render::CommandType::VerticesDraw) || (commandHeader->type == Render::CommandType::IndicesDraw))) {
			OpenGLPopVertexIndexArray(vertexIndexArrayStack);
			OpenGLCheckError();
		}

		switch (commandHeader->type) {
			case Render::CommandType::Viewport:
			{
				Render::Viewport *viewport = static_cast<Render::Viewport *>((void *)commandAt);
				glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
			} break;

			case Render::CommandType::OrthoProjection:
			{
				Render::OrthoProjection *ortho = static_cast<Render::OrthoProjection *>((void *)commandAt);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(ortho->left, ortho->right, ortho->bottom, ortho->top, ortho->nearClip, ortho->farClip);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
			} break;

			case Render::CommandType::Clear:
			{
				Render::Clear *clear = static_cast<Render::Clear *>((void *)commandAt);
				GLbitfield flags = 0;
				if (clear->isColor) {
					flags |= GL_COLOR_BUFFER_BIT;
				}
				if (clear->isDepth) {
					flags |= GL_DEPTH_BUFFER_BIT;
				}
				glClearColor(clear->color.r, clear->color.g, clear->color.b, clear->color.a);
				glClear(flags);
			} break;

			case Render::CommandType::Lines:
			{
				Render::Vertices *lines = static_cast<Render::Vertices *>((void *)commandAt);
				Vec2f *points = lines->points;
				glColor4fv(&lines->color.m[0]);
				glLineWidth(lines->lineWidth);
				glBegin(GL_LINES);
				for (uint32_t pointIndex = 0; pointIndex < lines->pointCount; ++pointIndex) {
					Vec2f point = points[pointIndex];
					glVertex2fv(&point.m[0]);
				}
				glEnd();
				glLineWidth(1.0f);
			} break;

			case Render::CommandType::Polygon:
			{
				Render::Vertices *verts = static_cast<Render::Vertices *>((void *)commandAt);
				Vec2f *points = verts->points;
				glColor4fv(&verts->color.m[0]);
				glLineWidth(verts->lineWidth);
				glBegin(verts->isFilled ? GL_POLYGON : GL_LINE_LOOP);
				for (uint32_t pointIndex = 0; pointIndex < verts->pointCount; ++pointIndex) {
					Vec2f point = points[pointIndex];
					glVertex2fv(&point.m[0]);
				}
				glEnd();
				glLineWidth(1.0f);
			} break;

			case Render::CommandType::Rectangle:
			{
				Render::Rectangle *rect = static_cast<Render::Rectangle *>((void *)commandAt);
				glColor4fv(&rect->color.m[0]);
				glLineWidth(rect->lineWidth);
				glBegin(rect->isFilled ? GL_QUADS : GL_LINE_LOOP);
				glVertex2f(rect->bottomLeft.x + rect->size.w, rect->bottomLeft.y + rect->size.h);
				glVertex2f(rect->bottomLeft.x, rect->bottomLeft.y + rect->size.h);
				glVertex2f(rect->bottomLeft.x, rect->bottomLeft.y);
				glVertex2f(rect->bottomLeft.x + rect->size.w, rect->bottomLeft.y);
				glEnd();
				glLineWidth(1.0f);
			} break;

			case Render::CommandType::Sprite:
			{
				Render::Sprite *sprite = static_cast<Render::Sprite *>((void *)commandAt);
				Vec2f pos = sprite->position;
				Vec2f size = sprite->size;
				GLuint textureHandle = PointerToValue<GLuint>(sprite->texture);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, textureHandle);
				glColor4fv(&sprite->color.m[0]);
				glBegin(GL_QUADS);
				glTexCoord2f(sprite->uvMax.x, sprite->uvMax.y);
				glVertex2f(pos.x + size.w, pos.y + size.h);
				glTexCoord2f(sprite->uvMin.x, sprite->uvMax.y);
				glVertex2f(pos.x, pos.y + size.h);
				glTexCoord2f(sprite->uvMin.x, sprite->uvMin.y);
				glVertex2f(pos.x, pos.y);
				glTexCoord2f(sprite->uvMax.x, sprite->uvMin.y);
				glVertex2f(pos.x + size.w, pos.y);
				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
			} break;

			case Render::CommandType::Circle:
			{
				const int segments = 16;
				const float segmentRad = ((float)M_PI * 2.0f) / (float)segments;
				Render::Circle *circle = static_cast<Render::Circle *>((void *)commandAt);
				glColor4fv(&circle->color.m[0]);
				glLineWidth(circle->lineWidth);
				glBegin(circle->isFilled ? GL_POLYGON : GL_LINE_LOOP);
				for (int segmentIndex = 0; segmentIndex < segments; ++segmentIndex) {
					float r = (float)segmentIndex * segmentRad;
					float x = circle->position.x + cosf(r) * circle->radius;
					float y = circle->position.y + sinf(r) * circle->radius;
					glVertex2f(x, y);
				}
				glEnd();
				glLineWidth(1.0f);
			} break;

			case Render::CommandType::VertexIndexHeader:
			{
				Render::VertexIndexArrayHeader *vertexIndexArray = static_cast<Render::VertexIndexArrayHeader *>((void *)commandAt);
				assert(vertexIndexArray->vertices != nullptr);
				OpenGLPushVertexIndexArray(vertexIndexArrayStack, vertexIndexArray);
			} break;

			case Render::CommandType::VerticesDraw:
			case Render::CommandType::IndicesDraw:
			{
				assert(vertexIndexArrayStack.size() > 0);
				Render::VertexIndexArrayHeader *vertexIndexArray = vertexIndexArrayStack.top();

				Render::VertexIndexArrayDraw *vertexIndexArrayDraw = static_cast<Render::VertexIndexArrayDraw *>((void *)commandAt);
				GLuint primitiveType;
				switch (vertexIndexArrayDraw->drawType) {
					case Render::PrimitiveType::Points:
					{
						primitiveType = GL_POINTS;
						float pointSize = vertexIndexArrayDraw->pointSize;
						glPointSize(pointSize);
					} break;
					case Render::PrimitiveType::Triangles:
					{
						primitiveType = GL_TRIANGLES;
					} break;
					default:
					{
						primitiveType = GL_TRIANGLES;
					} break;
				}

				if (vertexIndexArrayDraw->texture != nullptr) {
					GLuint textureId = PointerToValue<GLuint>(vertexIndexArrayDraw->texture);
					// @TODO: Dont want to enable blending here always, use attributes for that!
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, textureId);
				}

				if (commandHeader->type == Render::CommandType::VerticesDraw) {
					glDrawArrays(primitiveType, 0, (GLsizei)vertexIndexArrayDraw->count);
				} else {

					size_t indexSize = vertexIndexArray->indexSize;
					GLenum type = indexSize == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
					glDrawElements(primitiveType, (GLsizei)vertexIndexArrayDraw->count, type, vertexIndexArray->indices);
				}

				if (vertexIndexArrayDraw->texture != nullptr) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
				}

				if (vertexIndexArrayDraw->drawType == Render::PrimitiveType::Points) {
					glPointSize(1.0f);
				}
			} break;
		}

		OpenGLCheckError();

		commandAt += commandHeader->dataSize;
	}
	OpenGLPopVertexIndexArray(vertexIndexArrayStack);
}

int main(int argc, char **args) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.window.windowSize.width = kWindowWidth;
	settings.window.windowSize.height = kWindowHeight;
	settings.video.driver = fplVideoDriverType_OpenGL;
	fplFormatString(settings.window.title, fplArrayCount(settings.window.title), "NBody Simulation v%s", kAppVersion);
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		if (fglLoadOpenGL(true)) {
			Application *app = globalApp = new DemoApplication();
			ApplicationWindow *window = app->GetWindow();

			// @NOTE(final): Get window area at startup, because the titlebar and borders takes up space too.
			fplWindowSize windowArea;
			if (fplGetWindowSize(&windowArea)) {
				window->width = windowArea.width;
				window->height = windowArea.height;
			}

			app->Init();
			
			lastFrameClock = fplGetWallClock();

			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					switch (ev.type) {
						case fplEventType_Window:
						{
							switch (ev.window.type) {
								case fplWindowEventType_Resized:
								{
									globalApp->Resize(ev.window.size.width, ev.window.size.height);
								} break;
							}
						} break;
						case fplEventType_Keyboard:
						{
							switch (ev.keyboard.type) {
								case fplKeyboardEventType_Button:
								{
									if (ev.keyboard.buttonState == fplButtonState_Release) {
										globalApp->KeyUp(ev.keyboard.mappedKey);
									} else if (ev.keyboard.buttonState >= fplButtonState_Press) {
										globalApp->KeyDown(ev.keyboard.mappedKey);
									}
								} break;
							}
						} break;
					}
				}

				Render::ResetCommandBuffer(globalApp->commandBuffer);
				globalApp->UpdateAndRender(lastFrameTime, lastFrameCycles);
				OpenGLDrawCommandBuffer(globalApp->commandBuffer);

				fplVideoFlip();

				fplWallClock endFrameClock = fplGetWallClock();
				lastFrameTime = fplGetWallDelta(lastFrameClock, endFrameClock);
				lastFrameClock = endFrameClock;

				uint64_t endCycles = fplCPURDTSC();
				lastFrameCycles = endCycles - lastCycles;
				lastCycles = endCycles;
			}
			fglUnloadOpenGL();
			delete app;
		}
		fplPlatformRelease();
	}

	return 0;
}