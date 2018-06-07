#ifndef FINAL_RENDER_H
#define FINAL_RENDER_H

#include "final_vecmath.h"

struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
};

inline UVRect UVRectFromTile(const Vec2i &imageSize, const Vec2i &tileSize, const int border, const Vec2i &pos) {
	Vec2f texel = V2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	int imgX = border + pos.x * tileSize.x + border * pos.x;
	int imgY = border + pos.y * tileSize.y + border * pos.y;
	UVRect result;
	result.uMin = imgX * texel.x;
	result.vMin = imgY * texel.y;
	result.uMax = result.uMin + tileSize.x * texel.x;
	result.vMax = result.vMin + tileSize.y * texel.y;
	return(result);
}

inline UVRect UVRectFromPos(const Vec2i &imageSize, const Vec2i &partSize, const Vec2i &pos) {
	Vec2f texel = V2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
	return(result);
}

struct Viewport {
	int x;
	int y;
	int w;
	int h;
};

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect);

struct Camera2D {
	Vec2f offset;
	float worldToPixels;
	float pixelsToWorld;
	float scale;
};

#endif // FINAL_RENDER_H

#if defined(FINAL_RENDER_IMPLEMENTATION) && !defined(FINAL_RENDER_IMPLEMENTED)

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2i(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2i(0, 0);
	if(targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport result = { viewOffset.x, viewOffset.y, viewSize.w, viewSize.h };
	return(result);
}

#endif // FINAL_RENDER_IMPLEMENTATION