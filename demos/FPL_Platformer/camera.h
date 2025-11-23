#ifndef CAMERA_H
#define CAMERA_H

#include <final_platform_layer.h>
#include <final_math.h>
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

typedef struct Camera {
	// Camera transform (0 = Current, 1 = Last)
	CameraTransform transform[2];
	// Current camera limits
	CameraLimits limits;
	// Current camera target
	CameraTarget target;
	// Current camera view radius
	Vec2f viewRadius;
	// Factor that is used to convert from world coordinate to screen coordinates
	float worldToPixels;
	// Factor that is used to convert from screen coordinate to world coordinates
	float pixelsToWorld;
} Camera;

fpl_inline void CameraSetPos(Camera *camera, const Vec2f pos) {
	if (camera->limits.isEnabled) {
		Vec2f minPos = V2fAdd(camera->limits.bounds.min, camera->viewRadius);
		Vec2f maxPos = V2fSub(camera->limits.bounds.max, camera->viewRadius);
		Vec2f newPos = V2fClamp(pos, minPos, maxPos);
		camera->transform[0].offset = V2fNegate(newPos);
	} else {
		camera->transform[0].offset = V2fNegate(pos);
	}
}

#endif // CAMERA_H
