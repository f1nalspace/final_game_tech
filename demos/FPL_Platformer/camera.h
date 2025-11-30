#ifndef CAMERA_H
#define CAMERA_H

#include <final_platform_layer.h>
#include <final_math.h>
#include <final_render.h>
#include <final_geometry.h>

#define CameraMinMovementDelta 0.01f
#define CameraMovementDamping 0.999f

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
	Viewport4i viewport;
	// Current camera view radius in world coordinates
	Vec2f viewRadius;
	// Current velocity
	Vec2f velocity;
	// Target position in world coordinates
	Vec2f targetPos;
	// Factor that is used to convert from world coordinate to screen coordinates
	float worldToPixels;
	// Factor that is used to convert from screen coordinate to world coordinates
	float pixelsToWorld;
	// ID of the camera
	uint32_t id;
	// Speed to target in world units per second
	float targetSpeed;
	// Is target set
	bool hasTarget;
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

fpl_inline void CameraUpdateViewport(Camera *camera, const Viewport4i *viewport, const float worldWidth) {
	camera->viewport = *viewport;
	camera->worldToPixels = ((float)viewport->w / worldWidth) * camera->transform[0].scale;
	camera->pixelsToWorld = 1.0f / camera->worldToPixels;
}

fpl_inline void CameraSetScale(Camera *camera, const float scale, const bool reset) {
	camera->transform[0].scale = scale;
	if (reset)
		camera->transform[1].scale = camera->transform[0].scale;
}

fpl_inline Vec2f CameraGetLimitedPosition(const Camera *camera, const Vec2f pos) {
	Vec2f minPos = V2fAdd(camera->limits.bounds.min, camera->viewRadius);
	Vec2f maxPos = V2fSub(camera->limits.bounds.max, camera->viewRadius);
	Vec2f result = V2fClamp(pos, minPos, maxPos);
	return result;
}

fpl_inline void CameraSetPos(Camera *camera, const Vec2f pos, const bool reset) {
	if (camera->limits.isEnabled) {
		Vec2f newPos = CameraGetLimitedPosition(camera, pos);
		camera->transform[0].offset = V2fNegate(newPos);
	} else {
		camera->transform[0].offset = V2fNegate(pos);
	}
	if (reset) {
		camera->transform[1].offset = camera->transform[0].offset;
		camera->velocity = V2fZero();
	}
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

fpl_inline void CameraMoveTo(Camera *camera, const Vec2f target, const float speed) {
	Vec2f t = V2fMultScalar(target, 1.0f);
	Vec2f p = V2fSub(camera->transform[0].offset, t);
	float s = F32Abs(V2fLength(p) / speed);
	if (s > 1.01f) {
		Vec2f n = V2fNormalize(p);
		camera->velocity = V2fAddMultScalar(camera->velocity, n, -s);
	}
}

fpl_inline void CameraSetTarget(Camera *camera, const Vec2f targetPos, const float targetSpeed) {
	camera->targetPos = targetPos;
	camera->targetSpeed = targetSpeed;
	camera->hasTarget = true;
}

fpl_inline void CameraUpdate(Camera *camera, const float dt) {
	// Move towards the target, if needed
	if (camera->hasTarget) {
		CameraMoveTo(camera, camera->targetPos, camera->targetSpeed);
	}

	// Apply movement when the absolute speed is over the delta tolerance
	Vec2f offset = camera->transform[0].offset;
	if (F32Abs(camera->velocity.x) >= CameraMinMovementDelta) {
		offset.x += camera->velocity.x * dt;
		camera->velocity.x += CameraMovementDamping;
	}
	if (F32Abs(camera->velocity.y) >= CameraMinMovementDelta) {
		offset.y += camera->velocity.y * dt;
		camera->velocity.y += CameraMovementDamping;
	}

	// Dont allow camera offsets that are out of range of the limit area
	if (camera->limits.isEnabled) {
		camera->transform[0].offset = CameraGetLimitedPosition(camera, offset);
	} else {
		camera->transform[0].offset = offset;
	}

	// Stop movement when inside the delta tolerance
	if (F32Abs(camera->velocity.x) < CameraMinMovementDelta) {
		camera->velocity.x = 0.0f;
	}
	if (F32Abs(camera->velocity.y) < CameraMinMovementDelta) {
		camera->velocity.y = 0.0f;
	}
}

#endif // CAMERA_H
