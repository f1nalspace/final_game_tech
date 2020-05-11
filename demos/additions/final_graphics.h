/*
Name:
	Final Graphics

Description:
	Contains functions for drawing stuff like lines, directly into the backbuffer.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2020 Torsten Spaete
*/

#ifndef FINAL_GRAPHICS_H
#define FINAL_GRAPHICS_H

#include <final_platform_layer.h>

extern void BackbufferDrawLine(fplVideoBackBuffer *backBuffer, const float x0f, const float y0f, const float x1f, const float y1f, const uint32_t color);
extern void BackbufferDrawRect(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color);

#endif

#if defined(FINAL_GRAPHICS_IMPLEMENTATION) && !defined(FINAL_GRAPHICS_IMPLEMENTED)
#define FINAL_GRAPHICS_IMPLEMENTED

static int ClampBackBufferPosition(int value, int min, int max) {
	int result = value;
	if (result < min) result = min;
	if (result > max) result = max;
	return(result);
}

extern void BackbufferDrawLine(fplVideoBackBuffer *backBuffer, const float x0f, const float y0f, const float x1f, const float y1f, const uint32_t color) {
	int x0 = (int)(x0f + 0.5f);
	int y0 = (int)(y0f + 0.5f);
	int x1 = (int)(x1f + 0.5f);
	int y1 = (int)(y1f + 0.5f);

	int imageLine = backBuffer->width;

	int size = backBuffer->width * backBuffer->height;

	int dx = x1 - x0;
	int dy = y1 - y0;

	int dLong = abs(dx);
	int dShort = abs(dy);

	int offsetLong = dx > 0 ? 1 : -1;
	int offsetShort = dy > 0 ? imageLine : -imageLine;

	if (dLong < dShort) {
		int tmp[2] = { dShort, offsetShort };
		dShort = dLong;
		offsetShort = offsetLong;
		dLong = tmp[0];
		offsetLong = tmp[1];
	}

	int error = dLong / 2;
	int index = y0 * imageLine + x0;

	const int offset[] = { offsetLong, offsetLong + offsetShort };
	const int abs_d[] = { dShort, dShort - dLong };
	for (int i = 0; i <= dLong; ++i) {
		if (index >= 0 && index < size) {
			backBuffer->pixels[index] = color;
		}
		const int errorIsTooBig = error >= dLong;
		index += offset[errorIsTooBig];
		error += abs_d[errorIsTooBig];
	}
}

extern void BackbufferDrawRect(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color) {
	int minX = (int)(x0 + 0.5f);
	int minY = (int)(y0 + 0.5f);
	int maxX = (int)(x1 + 0.5f);
	int maxY = (int)(y1 + 0.5f);
	if (minX > maxX) {
		minX = (int)(x1 + 0.5f);
		maxX = (int)(x0 + 0.5f);
	}
	if (minY > maxY) {
		minY = (int)(y1 + 0.5f);
		maxY = (int)(y0 + 0.5f);
	}
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	minX = ClampBackBufferPosition(minX, 0, w - 1);
	maxX = ClampBackBufferPosition(maxX, 0, w - 1);
	minY = ClampBackBufferPosition(minY, 0, h - 1);
	maxY = ClampBackBufferPosition(maxY, 0, h - 1);
	for (int yp = minY; yp <= maxY; ++yp) {
		uint32_t *pixel = backBuffer->pixels + yp * backBuffer->width + minX;
		for (int xp = minX; xp <= maxX; ++xp) {
			*pixel++ = color;
		}
	}
}

#endif // FINAL_GRAPHICS_IMPLEMENTATION