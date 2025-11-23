#ifndef CAMERA_H
#define CAMERA_H

#include <final_platform_layer.h>
#include <final_math.h>
#include <final_geometry.h>

typedef struct CameraLimits {
	AABB2f bounds;
	bool isEnabled;
} CameraLimits;

typedef struct Camera {
	// Current camera limits
	CameraLimits limits;
	// Current camera view radius
	Vec2f viewRadius;
	// 0 = Current offset, 1 = Last offset
	Vec2f offset[2];
	// 0 = Current scale, 1 = Last scale
	float scale[2];
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
		camera->offset[0] = V2fNegate(newPos);
	} else {
		camera->offset[0] = V2fNegate(pos);
	}
}

#endif // CAMERA_H
