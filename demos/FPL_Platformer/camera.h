#ifndef CAMERA_H
#define CAMERA_H

#include <final_platform_layer.h>
#include <final_math.h>
#include <final_render.h>
#include <final_geometry.h>

typedef struct CameraLimits {
	AABB2f bounds;
	bool isEnabled;
} CameraLimits;

typedef struct CameraTarget {
	Vec2f pos;
	float speed;
	bool hasTarget;
} CameraTarget;

typedef struct CameraTransform {
	Vec2f offset;
	float scale;
} CameraTransform;

fpl_inline CameraTransform CameraTransformInit(const Vec2f offset, const float scale) {
	CameraTransform result = fplStructInit(CameraTransform, offset, scale);
	return result;
}

typedef struct Camera {
	// Camera transform (0 = Current, 1 = Last)
	CameraTransform transform[2];
	// Current camera limits
	CameraLimits limits;
	// Current camera target
	CameraTarget target;
	// Current viewport in screen space
	Viewport viewport;
	// Current camera view radius in world coordinates
	Vec2f viewRadius;
	// Factor that is used to convert from world coordinate to screen coordinates
	float worldToPixels;
	// Factor that is used to convert from screen coordinate to world coordinates
	float pixelsToWorld;
	// ID of the camera
	uint32_t id;
} Camera;

fpl_inline bool CameraInit(Camera *camera, const uint32_t id, const Vec2f offset, const float scale, const Vec2f viewRadius) {
	if (camera == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(camera);
	camera->id = id;
	camera->transform[0].offset = V2fNegate(offset);
	camera->transform[0].scale = scale;
	camera->transform[1] = camera->transform[0];
	camera->viewRadius = viewRadius;
	return true;
}

fpl_inline void CameraUpdateViewport(Camera *camera, const Viewport *viewport, const float worldWidth) {
	camera->viewport = *viewport;
	camera->worldToPixels = ((float)viewport->w / worldWidth) * camera->transform[0].scale;
	camera->pixelsToWorld = 1.0f / camera->worldToPixels;
}

fpl_inline void CameraSetScale(Camera *camera, const float scale, const bool reset) {
	camera->transform[0].scale = scale;
	if (reset)
		camera->transform[1].scale = camera->transform[0].scale;
}

fpl_inline void CameraSetPos(Camera *camera, const Vec2f pos, const bool reset) {
	if (camera->limits.isEnabled) {
		// Clamp position when limit bounds are enabled
		Vec2f minPos = V2fAdd(camera->limits.bounds.min, camera->viewRadius);
		Vec2f maxPos = V2fSub(camera->limits.bounds.max, camera->viewRadius);
		Vec2f newPos = V2fClamp(pos, minPos, maxPos);
		camera->transform[0].offset = V2fNegate(newPos);
	} else {
		camera->transform[0].offset = V2fNegate(pos);
	}
	if (reset)
		camera->transform[1].offset = camera->transform[0].offset;
}

fpl_inline void CameraZoomToPosition(Camera *camera, const float oldZoom, const float newZoom, const float worldWidth, const Vec2f target) {
	fplAssert(oldZoom > 0.0f && newZoom > 0.0f);
	fplAssert(worldWidth > 0.0f);

	// Old conversion factors
	float viewportWidth = (float)camera->viewport.w;
    float oldWorldToPixels = (viewportWidth / (float)worldWidth) * oldZoom;
    float newWorldToPixels = (viewportWidth / (float)worldWidth) * newZoom;

	// Screen position of target before zoom
    Vec2f offsetOld = camera->transform[0].offset;
    float screenX = target.x * oldWorldToPixels - offsetOld.x * oldWorldToPixels;
    float screenY = target.y * oldWorldToPixels - offsetOld.y * oldWorldToPixels;

	// New offset so target projects to same screen position
	Vec2f offsetNew = V2fInit(target.x - screenX / newWorldToPixels, target.y - screenY / newWorldToPixels);

	// NOTE(final): Do not use CameraSetPos() or CameraSetScale()
	camera->transform[0].scale = newZoom;
	camera->transform[0].offset = offsetNew;
}

#endif // CAMERA_H
