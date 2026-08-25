/***
fui_backend_gl1.h

--- About ---

A reference render backend for final_ui.h on LEGACY OpenGL: fixed function, client arrays, no shader and
no buffer object. It is the smallest backend that can exist, which is exactly why it is the one to read
first -- draining fuiDrawData is about forty lines, and everything else here is state setup.

final_ui.h emits raw pixel coordinates and hands every command its already-resolved clip rectangle, so a
backend needs to do only three things per command: set the scissor, bind a texture or none, and draw the
indices. There is no matrix in the draw data on purpose; installing the projection is the backend's job,
and it is the one place the y-down convention has to be turned back into whatever the API wants.

--- Getting started ---

- Include this AFTER a header that declares the OpenGL 1.1 entry points (final_dynamic_opengl.h will do)
- Define FUI_GL1_IMPLEMENTATION in ONE translation unit before including it
- Upload the font atlas once with fuiGL1UploadFontAtlas, and pass the texture it returns to the fuiFont

--- Usage ---

	uint32_t atlasTexture = 0;
	fuiGL1UploadFontAtlas(bakedFont.atlasPixels, bakedFont.atlasWidth, bakedFont.atlasHeight, &atlasTexture);
	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, (fuiTextureId)atlasTexture);
	...
	fuiEndFrame(&context);
	fuiGL1Render(fuiGetDrawData(&context));
	fplVideoFlip();

--- License ---

MIT License, Copyright (c) 2017-2026 Torsten Spaete
***/

#ifndef FUI_BACKEND_GL1_INCLUDE_H
#define FUI_BACKEND_GL1_INCLUDE_H

#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Uploads a one channel coverage atlas as a texture this backend can draw text with.
* @param[in] alphaPixels One byte of coverage per texel, width * height of them.
* @param[in] width Width of the atlas in texels.
* @param[in] height Height of the atlas in texels.
* @param[out] outTexture Receives the OpenGL texture name.
* @return Returns true when the texture was created.
* @note The atlas is EXPANDED to luminance-alpha on the way in, with the luminance forced to white. A
*       one channel GL_ALPHA texture would modulate the vertex color's rgb down to black, which is a
*       classic way to end up with invisible text and no idea why.
*/
fui_api bool fuiGL1UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture);

/**
* @brief Deletes a texture created by @ref fuiGL1UploadFontAtlas.
* @param[in] texture The OpenGL texture name.
*/
fui_api void fuiGL1DeleteTexture(const uint32_t texture);

/**
* @brief Draws one finished frame of user interface.
* @param[in] drawData The draw data from @ref fuiGetDrawData.
* @note Saves and restores nothing: it assumes it owns the pipeline for the length of the call, which is
*       what a UI drawn last over a finished scene actually does.
*/
fui_api void fuiGL1Render(const fuiDrawData *drawData);

#ifdef __cplusplus
}
#endif

#endif // FUI_BACKEND_GL1_INCLUDE_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
#if defined(FUI_GL1_IMPLEMENTATION) && !defined(FUI_GL1_IMPLEMENTED)
#define FUI_GL1_IMPLEMENTED

#include <stdlib.h>

fui_api bool fuiGL1UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture) {
	if(alphaPixels == fui_null || outTexture == fui_null || width == 0 || height == 0) {
		return(false);
	}

	// Luminance white, alpha from the bake. Modulated against the vertex color this gives rgb = the text
	// color and a = the text alpha times the glyph's coverage, which is what anti-aliased text needs.
	size_t texelCount = (size_t)width * (size_t)height;
	unsigned char *luminanceAlpha = (unsigned char *)malloc(texelCount * 2u);
	if(luminanceAlpha == fui_null) {
		return(false);
	}
	for(size_t texelIndex = 0; texelIndex < texelCount; ++texelIndex) {
		luminanceAlpha[texelIndex * 2u + 0u] = 255;
		luminanceAlpha[texelIndex * 2u + 1u] = alphaPixels[texelIndex];
	}

	GLuint textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	// Rows are byte tight rather than the four byte default, or every atlas whose width is not a multiple
	// of two would come out sheared.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, (GLsizei)width, (GLsizei)height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, luminanceAlpha);
	// Linear, because every glyph on screen is a REDUCTION of the baked size and nearest would alias it
	// to pieces. Clamped, so the edge texel of one glyph never bleeds into the next one.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	free(luminanceAlpha);
	*outTexture = (uint32_t)textureName;
	return(textureName != 0);
}

fui_api void fuiGL1DeleteTexture(const uint32_t texture) {
	if(texture != 0) {
		GLuint textureName = (GLuint)texture;
		glDeleteTextures(1, &textureName);
	}
}

fui_api void fuiGL1Render(const fuiDrawData *drawData) {
	if(drawData == fui_null || drawData->commandCount == 0 || drawData->vertexCount == 0) {
		return;
	}

	GLsizei windowWidth = (GLsizei)drawData->windowSize.x;
	GLsizei windowHeight = (GLsizei)drawData->windowSize.y;
	if(windowWidth <= 0 || windowHeight <= 0) {
		return;
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// The ONE place the convention is turned around: final_ui.h works y-down from the top left, and this
	// ortho puts the top of the window at y = 0 by passing bottom and top the other way round.
	glOrtho(0.0, (double)windowWidth, (double)windowHeight, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	const fuiVertex *vertices = drawData->vertices;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, (GLsizei)sizeof(fuiVertex), &vertices->position);
	glTexCoordPointer(2, GL_FLOAT, (GLsizei)sizeof(fuiVertex), &vertices->uv);
	// The packed color is byte order red, green, blue, alpha on a little endian machine, which is exactly
	// what GL_UNSIGNED_BYTE wants. That is why final_ui.h packs it the way it does.
	glColorPointer(4, GL_UNSIGNED_BYTE, (GLsizei)sizeof(fuiVertex), &vertices->color);

	uint32_t boundTexture = 0;
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	// The scissor is set only when it CHANGES. A run of commands inside one panel all carry the same clip,
	// and a dense interface is mostly such runs, so this is most of the scissor calls gone for the price
	// of four comparisons. The first command always sets it, because nothing has been set yet.
	GLint lastScissorX = 0;
	GLint lastScissorY = 0;
	GLsizei lastScissorWidth = -1;
	GLsizei lastScissorHeight = -1;

	for(uint32_t commandIndex = 0; commandIndex < drawData->commandCount; ++commandIndex) {
		const fuiDrawCommand *command = &drawData->commands[commandIndex];
		if(command->indexCount == 0) {
			continue;
		}

		// The clip rectangle rides along on every command already intersected with everything enclosing it,
		// so there is no clip stack to keep here. The scissor box counts from the BOTTOM of the window.
		fuiRect clip = command->clipRect;
		GLint scissorX = (GLint)clip.x;
		GLint scissorY = (GLint)((float)windowHeight - (clip.y + clip.h));
		GLsizei scissorWidth = (GLsizei)clip.w;
		GLsizei scissorHeight = (GLsizei)clip.h;
		if(scissorWidth <= 0 || scissorHeight <= 0) {
			continue;
		}
		bool scissorChanged = (scissorX != lastScissorX) || (scissorY != lastScissorY) || (scissorWidth != lastScissorWidth) || (scissorHeight != lastScissorHeight);
		if(scissorChanged) {
			glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
			lastScissorX = scissorX;
			lastScissorY = scissorY;
			lastScissorWidth = scissorWidth;
			lastScissorHeight = scissorHeight;
		}

		uint32_t wantedTexture = (uint32_t)command->texture;
		if(wantedTexture != boundTexture) {
			if(wantedTexture != 0) {
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, (GLuint)wantedTexture);
			} else {
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			boundTexture = wantedTexture;
		}

		// Every command's kind is ignored here: the indices describe the same geometry either way, and a
		// backend only reaches for the fast path payloads when it has a cheaper call to make than triangles.
		const fuiDrawIndex *indices = drawData->indices + command->indexOffset;
#if FUI_USE_16BIT_INDICES
		glDrawElements(GL_TRIANGLES, (GLsizei)command->indexCount, GL_UNSIGNED_SHORT, indices);
#else
		glDrawElements(GL_TRIANGLES, (GLsizei)command->indexCount, GL_UNSIGNED_INT, indices);
#endif
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif // FUI_GL1_IMPLEMENTATION
