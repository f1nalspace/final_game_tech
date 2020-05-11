/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Raytracer

Description:
	Very simple multi-threaded 3D Raytracer demo.
	* Inspired by handmade ray (Casey Muratori)

	Right know, we dont do any physical correct lighting whatsoever.

	The point of this demo, is to test multithreading and software video output.
	Also there are defines, which can be toggled to enable/disable false sharing or compiler reordering issues.

Todo:
	- Better random
	- Fix bad random bounce
	- Fix non physically corrected shading
	- Blitting of raytracing image to the backbuffer with different sizes
	- Lights
	- Box Shape
	- Triangle Shape
	- SIMD?

Requirements:
	- C++/11 Compiler
	- Final Platform Layer
	- Final Dynamic OpenGL (Only for 3D preview)
	- GLM (Only for preview)

Author:
	Torsten Spaete

Changelog:
	## 2019-08-09
	- Fixed false sharing issues for work queue

	## 2019-06-01
	- Initial version

License:
	Copyright (c) 2017-2020 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Define this to enable OpenGL preview, instead of doing the actual raytracing
#define USE_OPENGL_NO_RAYTRACE 0

// If one of these are disabled, we may get false sharing issues
// -> Resulting in crashes
#define QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE 1
#define QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES 1

// If this is disabled we may get null pointers, due to bad compiler read-instructions reorderings
// -> Resulting in crashes
#define FIX_WRONG_INSTRUCTION_REORDER_IN_RELEASE 1

#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

// Custom types to save a bit of typing :D
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef float f32;
typedef int32_t b32;

#define U8_MAX UCHAR_MAX
#define U32_MAX UINT32_MAX
#define S32_MAX INT32_MAX
#define F32_MAX FLT_MAX

#if USE_OPENGL_NO_RAYTRACE
#	define FGL_IMPLEMENTATION
#	include <final_dynamic_opengl.h>
#endif

#include <final_math.h>
#include <final_geometry.h>
#include <final_random.h>

#include <vector>
#include <new>

struct Image32 {
	Pixel *pixels;
	u32 width;
	u32 height;

	inline void Fill(const Pixel &color) {
		for (u32 i = 0; i < width * height; ++i)
			pixels[i] = color;
	}
};

static Vec3f UnitRight = V3fInit(1, 0, 0);
static Vec3f UnitUp = V3fInit(0, 0, 1);
static Vec3f UnitForward = V3fInit(0, 1, 0);

enum class ObjectKind : s32 {
	None = 0,
	Plane,
	Sphere,
};

struct Material {
	Vec3f emitColor;
	Vec3f reflectColor;
	f32 scatter;
};

struct Object {
	union {
		Plane3f plane;
		Sphere3f sphere;
	};
	ObjectKind kind;
	u32 materialIndex;
};

struct Camera {
	Vec3f eye;
	Vec3f target;
	Vec3f up;
	f32 fov;
	f32 zNear;
	f32 zFar;
};

struct Scene {
	Camera camera;

	std::vector<Object> objects;
	std::vector<Material> materials;

	u32 AddMaterial(const Vec3f &emitColor, const Vec3f &reflectColor, const float scatter = 0.0f) {
		fplAssert(materials.size() < (U32_MAX - 1));
		u32 result = (u32)materials.size();
		Material mat = {};
		mat.emitColor = emitColor;
		mat.reflectColor = reflectColor;
		mat.scatter = scatter;
		materials.push_back(mat);
		return(result);
	}

	void AddPlane(const Vec3f &normal, const f32 distance, const u32 matIndex) {
		fplAssert(matIndex < materials.size());
		Object obj = {};
		obj.kind = ObjectKind::Plane;
		obj.plane.normal = normal;
		obj.plane.distance = distance;
		obj.materialIndex = matIndex;
		objects.push_back(obj);
	}

	void AddSphere(const Vec3f &origin, const f32 radius, const u32 matIndex) {
		fplAssert(matIndex < materials.size());
		Object obj = {};
		obj.kind = ObjectKind::Sphere;
		obj.sphere.origin = origin;
		obj.sphere.radius = radius;
		obj.materialIndex = matIndex;
		objects.push_back(obj);
	}
};

struct RaytracerSettings {
	u32 maxBounceCount;
	u32 raysPerPixelCount;
};

struct Raytracer {
	Image32 image;
	RaytracerSettings settings;
	RandomSeries rnd;
	Vec2f halfPixelSize;
};

struct App {
	Scene scene;
	Raytracer raytracer;
};

#if USE_OPENGL_NO_RAYTRACE
static void DrawSphere(const Vec3f &origin, f32 radius, int steps = 20) {
	f32 x, y, z;
	for (f32 alpha = 0.0; alpha < Pi32; alpha += Pi32 / steps) {
		glBegin(GL_TRIANGLE_STRIP);
		for (f32 beta = 0.0; beta < 2.01 * Pi32; beta += Pi32 / steps) {
			x = origin.x + radius * cos(beta)*sin(alpha + Pi32 / steps);
			y = origin.y + radius * sin(beta)*sin(alpha + Pi32 / steps);
			z = origin.z + radius * cos(alpha + Pi32 / steps);
			glVertex3f(x, y, z);

			x = origin.x + radius * cos(beta)*sin(alpha);
			y = origin.y + radius * sin(beta)*sin(alpha);
			z = origin.z + radius * cos(alpha);
			glVertex3f(x, y, z);
		}
		glEnd();
	}

}

static void DrawPlane(const Vec3f &normal, const f32 distance, f32 infiniteSize) {
	Vec3f u = V3fNormalize(V3fCross(normal, UnitRight));
	Vec3f v = V3fNormalize(V3fCross(normal, u));
	Vec3f p0 = normal * -distance;
	Vec3f fu = u * infiniteSize;
	Vec3f fv = v * infiniteSize;
	Vec3f p1 = p0 - fu - fv;
	Vec3f p2 = p0 + fu - fv;
	Vec3f p3 = p0 + fu + fv;
	Vec3f p4 = p0 - fu + fv;

	glBegin(GL_TRIANGLES);
	glVertex3fv(&p1.m[0]); // Top-right
	glVertex3fv(&p4.m[0]); // Top-left
	glVertex3fv(&p3.m[0]); // Bottom-left
	glVertex3fv(&p3.m[0]); // Bottom-left
	glVertex3fv(&p2.m[0]); // Bottom-right
	glVertex3fv(&p1.m[0]); // Top-right
	glEnd();
}

static void DrawCube(const Vec3f &pos, const f32 radius) {
	static f32 cubeVertices[] =
	{
		// Front
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		// Right
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		// Back
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		// Left
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		// Top
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		// Bottom
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f
	};

	glBegin(GL_TRIANGLES);
	for (int side = 0; side < 6; ++side) {
		for (int vertexIndex = 0; vertexIndex < 6; ++vertexIndex) {
			const f32 *v = &cubeVertices[side * 6 * 3 + vertexIndex * 3];
			f32 x = pos.x + v[0] * radius;
			f32 y = pos.y + v[1] * radius;
			f32 z = pos.z + v[2] * radius;
			glVertex3f(x, y, z);
		}
	}
	glEnd();
}
#endif

static void Render(const App &app) {
	fplWindowSize size = {};
	fplGetWindowSize(&size);

#if USE_OPENGL_NO_RAYTRACE
	const f32 aspect = size.height > 0 ? size.width / (f32)size.height : 1.0f;
	const bool wireframe = false;

	const Scene &scene = app.scene;

	Vec3f camEye = scene.camera.eye;
	Vec3f camTarget = scene.camera.target;
	Vec3f camUp = scene.camera.up;

	f32 fov = scene.camera.fov;
	f32 zNear = scene.camera.zNear;
	f32 zFar = scene.camera.zFar;

	glViewport(0, 0, size.width, size.height);

	Mat4f projMat = Mat4PerspectiveRH(fov, aspect, zNear, zFar);
	Mat4f viewMat = Mat4LookAtRH(camEye, camTarget, camUp);
	Mat4f viewProjMat = projMat * viewMat;
	glLoadMatrixf(&viewProjMat.m[0]);

	const Material &defaultMat = scene.materials[0];

	glClearColor(defaultMat.emitColor.r, defaultMat.emitColor.g, defaultMat.emitColor.b, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

#if 1
	const f32 infinityPlaneSize = 100.0f;
	for (const Object &obj : scene.objects) {
		const Material &mat = scene.materials[obj.materialIndex];
		glColor3fv(&mat.reflectColor.m[0]);
		switch (obj.kind) {
			case ObjectKind::Plane:
				DrawPlane(obj.plane.normal, obj.plane.distance, infinityPlaneSize);
				break;
			case ObjectKind::Sphere:
				DrawSphere(obj.sphere.origin, obj.sphere.radius);
				break;
		}
	}
#endif

#if 0
	// Coordinate cross
	Vec3f origin = V3f(0);
	f32 crossLen = 3.0f;
	Vec3f crossRight = origin + UnitRight * crossLen;
	Vec3f crossUp = origin + UnitUp * crossLen;
	Vec3f crossForward = origin + UnitForward * crossLen;
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	glColor3f(1, 0.2f, 0);	glVertex3fv(&crossRight.m[0]);
	glColor3f(1, 0.2f, 0);	glVertex3fv(&origin.m[0]);
	glColor3f(0, 1, 0.2f);	glVertex3fv(&crossUp.m[0]);
	glColor3f(0, 1, 0.2f);	glVertex3fv(&origin.m[0]);
	glColor3f(0.2f, 0, 1);	glVertex3fv(&crossForward.m[0]);
	glColor3f(0.2f, 0, 1);	glVertex3fv(&origin.m[0]);
	glEnd();
	glLineWidth(1.0f);

	// Coordinate cube
	glColor3f(0.2f, 0.2f, 0.2f);
	DrawCube(V3f(0, 0, 0), 0.5f);
	glColor3f(1.0f, 0.0f, 0.0f);
	DrawCube(V3f(1, 0, 0), 0.5f);
	glColor3f(0.0f, 1.0f, 0.0f);
	DrawCube(V3f(0, 1, 0), 0.5f);
	glColor3f(0.0f, 0.0f, 1.0f);
	DrawCube(V3f(0, 0, 1), 0.5f);
#endif

	Vec3f normal = V3fNormalize(camTarget - camEye);

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#else // !USE_OPENGL_NO_RAYTRACE
	fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();

	const Image32 &raytraceImage = app.raytracer.image;
	const u32 sourceLineWidth = raytraceImage.width * 4;

	// @TODO(final): Support to blit any arbitary sized image to the backbuffer
	// Right know we copy over each pixel from the raytraced image pixels, requiring that backbuffer size must equals the render image size
	fplAssert(backBuffer->width == raytraceImage.width);
	fplAssert(backBuffer->height == raytraceImage.height);

	for (u32 y = 0; y < backBuffer->height; ++y) {
		u32 *targetRow = (u32 *)((u8 *)backBuffer->pixels + (y * backBuffer->lineWidth));
		Pixel *sourceRow = raytraceImage.pixels + (y * raytraceImage.width);
		for (u32 x = 0; x < backBuffer->width; ++x) {
			Pixel sourcePixel = *sourceRow++;
			u32 color = BGRA8FromPixel(sourcePixel);
			*targetRow++ = color;
		}
	}
#endif // USE_OPENGL_NO_RAYTRACE
}

struct TilingInfo {
	u32 tileCountX;
	u32 tileCountY;
	u32 tileSizeX;
	u32 tileSizeY;
	u32 imageW;
	u32 imageH;
};

enum class WorkerState : s32 {
	Stopped = 0,
	Running,
};

struct WorkOrder {
	const Scene *scene;
	Raytracer *raytracer;
#if QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE == 1 && defined(FPL_CPU_32BIT)
	u8 padding1[8];
#endif
	u32 xMin;
	u32 yMin;
	u32 xMaxPlusOne;
	u32 yMaxPlusOne;
#if QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE == 1
	u8 padding2[32];
#endif
};

#if QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE == 1
fplStaticAssert(sizeof(WorkOrder) % 64 == 0);
#endif

struct WorkQueue {
	// @NOTE(final): Memory must be be aligned by 64-bit, otherwise we get false sharing issues.
	WorkOrder *orders;
	u32 capacity;
	u32 workOrderCount;

#if QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES == 1
	u8 cacheline_padding1[64];
#endif

	volatile u32 nextWorkOrderIndex;
#if QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES == 1
	u8 cacheline_padding2[64];
#endif

	volatile u32 completionCount;
#if QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES == 1
	u8 cacheline_padding3[64];
#endif

	bool IsEmpty() {
		bool result = workOrderCount == 0;
		return(result);
	}

	bool IsFinished() {
		bool result = false;
		if (workOrderCount > 0)
			result = fplAtomicLoadU32(&completionCount) == workOrderCount;
		return(result);
	}

	void Init(const u32 capacity) {
#if QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE == 1
		orders = (WorkOrder *)fplMemoryAlignedAllocate(capacity * sizeof(*orders), 64);
#else
		orders = (WorkOrder *)fplMemoryAllocate(capacity * sizeof(*orders));
#endif
		this->capacity = capacity;
		workOrderCount = 0;
		fplAtomicExchangeU32(&nextWorkOrderIndex, 0);
		fplAtomicExchangeU32(&completionCount, 0);
	}

	void Release() {
#if QUEUE_ALIGN_WORK_ORDERS_BY_CACHELINE == 1
		fplMemoryAlignedFree(orders);
#else
		fplMemoryFree(orders);
#endif
	}

	void Reset() {
		workOrderCount = 0;
		fplAtomicExchangeU32(&nextWorkOrderIndex, 0);
		fplAtomicExchangeU32(&completionCount, 0);
	}

	void Push(Raytracer *rayTracer, const Scene *scene, const u32 xMin, const u32 yMin, const u32 xMaxPlusOne, const u32 yMaxPlusOne) {
		const fplThreadHandle *mainThread = fplGetMainThread();
		u32 threadId = fplGetCurrentThreadId();
		fplAssert(threadId == mainThread->id);
		fplAssert(workOrderCount < capacity);
		u32 index = workOrderCount++;
		WorkOrder *order = orders + index;
		*order = {};
		order->raytracer = rayTracer;
		order->scene = scene;
		order->xMin = xMin;
		order->yMin = yMin;
		order->xMaxPlusOne = xMaxPlusOne;
		order->yMaxPlusOne = yMaxPlusOne;
	}

	bool Pop(u32 &outIndex) {
		if (IsEmpty() || fplAtomicLoadU32(&nextWorkOrderIndex) >= workOrderCount) {
			return(false);
		}
		u32 index = fplAtomicFetchAndAddU32(&nextWorkOrderIndex, 1);
		fplAssert(index < workOrderCount);
		outIndex = index;
		return(true);
	}
};

struct Worker {
	WorkQueue *queue;
	fplThreadHandle *thread;

#if QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES == 1
	u8 cacheline_padding1[64];
#endif
	volatile WorkerState state;
#if QUEUE_ADD_CACHELINE_PADDING_TO_VOLATILES == 1
	u8 cacheline_padding2[64];
#endif

	fplMutexHandle lockMutex;
	fplConditionVariable nonEmptyCondition;

	inline void Start() {
		fplAtomicExchangeS32((volatile s32 *)&state, (s32)WorkerState::Running);
	}

	inline void Stop() {
		fplAtomicExchangeS32((volatile s32 *)&state, (s32)WorkerState::Stopped);
	}

	inline bool IsStopped() const {
		WorkerState newState = (WorkerState)fplAtomicLoadS32((volatile s32 *)&state);
		bool result = (newState == WorkerState::Stopped);
		return(result);
	}
};

static bool RayPlaneIntersection(const Ray3f &ray, const Plane3f &plane, f32 &out, const float tolerance) {
	f32 denom = V3fDot(plane.normal, ray.direction);
	if ((denom < -tolerance) || (denom > tolerance)) {
		f32 t = (-plane.distance - V3fDot(plane.normal, ray.origin)) / denom;
		out = t;
		return(true);
	}
	return(false);
}

static bool RaySphereIntersection(const Ray3f &ray, const Sphere3f &sphere, f32 &out, const float tolerance) {
	Vec3f rayRelativeOrigin = ray.origin - sphere.origin;
	f32 a = V3fDot(ray.direction, ray.direction);
	f32 b = 2.0f * V3fDot(ray.direction, rayRelativeOrigin);
	f32 c = V3fDot(rayRelativeOrigin, rayRelativeOrigin) - sphere.radius * sphere.radius;

	f32 radiusSquared = sphere.radius * sphere.radius;
	f32 relativeDistanceSquared = V3fDot(rayRelativeOrigin, rayRelativeOrigin);

	f32 denom = 2.0f * a;
	f32 rootTerm = SquareRoot(b * b - 4.0f * a * c);
	if (rootTerm > tolerance) {
		f32 tPositive = (-b + rootTerm) / denom;
		f32 tNegative = (-b - rootTerm) / denom;

		f32 t = tPositive;
		if ((tNegative > 0.0f) && (tNegative < tPositive)) {
			t = tNegative;
		}

		out = t;
		return(true);
	}

	return(false);
}

// @NOTE(final): "Order" must be volatile, otherwise the compile may reorder instructions here
#if FIX_WRONG_INSTRUCTION_REORDER_IN_RELEASE
static bool RaytracePart(Worker &worker, volatile WorkOrder &order) {
#else
static bool RaytracePart(Worker &worker, WorkOrder &order) {
#endif
	const Scene *scene = order.scene;
	Raytracer *raytracer = order.raytracer;
	fplAssert(scene != fpl_null);
	fplAssert(raytracer != fpl_null);

	Image32 &image = raytracer->image;

	const f32 fov = scene->camera.fov;
	const f32 halfTan = Tan(fov * 0.5f);
	const f32 aspectRatio = (f32)image.width / (float)image.height;
	const Vec3f cameraPosition = scene->camera.eye;
	const Vec3f cameraUp = scene->camera.up;
	const Vec3f cameraTarget = scene->camera.target;

	// Construct camera axis
	const Vec3f cameraZ = V3fNormalize(cameraPosition - cameraTarget);
	const Vec3f cameraX = V3fNormalize(V3fCross(cameraUp, cameraZ));
	const Vec3f cameraY = V3fNormalize(V3fCross(cameraZ, cameraX));

	const Vec2f halfPixelSize = raytracer->halfPixelSize;

	f32 filmDistance = 1.0f;
	Vec3f filmCenter = cameraPosition - filmDistance * cameraZ;

	u32 raysPerPixel = raytracer->settings.raysPerPixelCount;
	u32 maxBounceCount = raytracer->settings.maxBounceCount;

	f32 contrib = 1.0f / (f32)raysPerPixel;

	fplAssert(scene->materials.size() > 0);
	const Material &defaultMaterial = scene->materials[0];

	for (u32 y = order.yMin; y < order.yMaxPlusOne; ++y) {
		u32 inverseY = (image.height - 1 - y);

		Pixel *row = &image.pixels[inverseY * image.width];

		f32 ratioY = (f32)y / (f32)image.height;
		f32 filmY = -1.0f + 2.0f * ratioY;

		Pixel *col = row + order.xMin;
		for (u32 x = order.xMin; x < order.xMaxPlusOne; ++x) {
			if (worker.IsStopped())
				return(false);

			f32 ratioX = (f32)x / (f32)image.width;
			f32 filmX = -1.0f + 2.0f * ratioX;

			Vec3f finalColor = {};

			for (u32 rayIndex = 0; rayIndex < raysPerPixel; ++rayIndex) {
				if (worker.IsStopped())
					return(false);

				f32 offsetX = RandomBilateral(&raytracer->rnd) * halfPixelSize.w;
				f32 offsetY = RandomBilateral(&raytracer->rnd) * halfPixelSize.h;

				f32 perspectiveX = (filmX + offsetX) * halfTan * aspectRatio;
				f32 perspectiveY = (filmY + offsetY) * halfTan;

				Vec3f filmP = filmCenter + (perspectiveX * cameraX) + (perspectiveY * cameraY);

				Vec3f rayOrigin = cameraPosition;
				Vec3f rayDirection = V3fNormalize(filmP - cameraPosition);

				Ray3f ray = MakeRay(rayOrigin, rayDirection);

				const f32 tolerance = 1e-6f;
				f32 minHitDistance = 0.0f;

				Vec3f sample = {};
				Vec3f attenuation = V3fInit(1, 1, 1);

				for (u32 bounceIndex = 0; bounceIndex < maxBounceCount; ++bounceIndex) {
					if (worker.IsStopped())
						return(false);

					f32 hitDistance = F32_MAX;
					u32 hitMaterialIndex = 0;
					Vec3f hitNormal = V3fZero();

					for (u32 objectIndex = 0, objectCount = (u32)scene->objects.size(); objectIndex < objectCount; ++objectIndex) {
						if (worker.IsStopped())
							return(false);

						const Object *obj = &scene->objects[objectIndex];

						f32 t = -F32_MAX;
						bool isHit = false;
						if (obj->kind == ObjectKind::Plane) {
							if (RayPlaneIntersection(ray, obj->plane, t, tolerance)) {
								if ((t > minHitDistance) && (t < hitDistance)) {
									hitDistance = t;
									hitMaterialIndex = obj->materialIndex;
									hitNormal = obj->plane.normal;
								}
							}
						} else if (obj->kind == ObjectKind::Sphere) {
							if (RaySphereIntersection(ray, obj->sphere, t, tolerance)) {
								if ((t > minHitDistance) && (t < hitDistance)) {
									hitDistance = t;
									hitMaterialIndex = obj->materialIndex;
									Vec3f relativeOrigin = ray.origin - obj->sphere.origin;
									hitNormal = V3fNormalize(t * ray.direction + relativeOrigin);
								}
							}
						}
					}

					if (hitMaterialIndex) {
						fplAssert(hitMaterialIndex < scene->materials.size());
						const Material &hitMaterial = scene->materials[hitMaterialIndex];

						sample += V3fHadamard(attenuation, hitMaterial.emitColor);

						f32 cosineAttenuation = V3fDot(-ray.direction, hitNormal);
						if (cosineAttenuation < 0) {
							cosineAttenuation = 0;
						}
						attenuation = V3fHadamard(attenuation, cosineAttenuation * hitMaterial.reflectColor);

						Vec3f pureBounce = ray.direction - 2.0f * V3fDot(ray.direction, hitNormal) * hitNormal;

#if 1
						// @NOTE(final): This is NOT a proper way to produce a random bounce. Do proper distribution based bounce
						Vec3f randomAddon = V3fInit(RandomBilateral(&raytracer->rnd), RandomBilateral(&raytracer->rnd), RandomBilateral(&raytracer->rnd));
						Vec3f randomBounce = V3fNormalize(hitNormal + randomAddon);
#else
						Vec3f randomBounce = RandomUnitHemisphere(&raytracer.rnd);
#endif

						// Ray for next bounce
						ray.origin += hitDistance * ray.direction;
						ray.direction = V3fNormalize(V3fLerp(randomBounce, hitMaterial.scatter, pureBounce));
					} else {
						sample += V3fHadamard(attenuation, defaultMaterial.emitColor);
						break;
					}
				}

				finalColor += contrib * sample;
			}

			Pixel outputPixel = LinearToPixelSRGB(V4fInitXYZ(finalColor, 1.0f));
			*col = outputPixel;
			++col;
		}

		++row;

		if (worker.IsStopped())
			return(false);
	}

	return(true);
}



#if USE_OPENGL_NO_RAYTRACE
static void InitGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.0f);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Left-handed coordinate system
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Cull back faces
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glMatrixMode(GL_MODELVIEW);
}
#endif

static void InitScene(Scene &scene) {
	scene.camera.eye = V3fInit(0, -10, 1);
	scene.camera.target = V3fInit(0, 0, 0);
	scene.camera.up = UnitUp;
	scene.camera.fov = DegreesToRadians(15.0f);
	scene.camera.zNear = 0.5f;
	scene.camera.zFar = 100.0f;

	scene.AddMaterial(V3fInit(0.152f, 0.22745f, 0.3647f), {});

	u32 floorMat = scene.AddMaterial(V3fInit(0, 0.0f, 0), V3fInit(0.1f, 0.5f, 0.1f), 0.75f);
	u32 whiteMat = scene.AddMaterial(V3fInit(0.0f, 0.0f, 0.0f), V3fInit(1.0f, 1.0f, 1.0f), 1.0f);
	u32 redMat = scene.AddMaterial(V3fInit(0.25f, 0.0f, 0.0f), V3fInit(1.0f, 0.0f, 0.0f), 1.0);
	u32 blueMat = scene.AddMaterial(V3fInit(0.0f, 0.0f, 0.25f), V3fInit(0.0f, 0.0f, 1.0f), 1.0f);

	scene.AddPlane(V3fInit(0, 0, 1), 0.0f, floorMat);
	scene.AddSphere(V3fInit(0, 0, 0.25f), 1.0f, whiteMat);
	scene.AddSphere(V3fInit(1, -2, 0.3f), 0.5f, redMat);
	scene.AddSphere(V3fInit(-1.0f, -0.75f, 0.9f), 0.3f, blueMat);
}

static void InitRaytracer(Raytracer &raytracer, const u32 raytraceWidth, const u32 raytraceHeight) {
	Image32 &raytraceImage = raytracer.image;
	raytraceImage.width = raytraceWidth;
	raytraceImage.height = raytraceHeight;
	raytraceImage.pixels = (Pixel *)fplMemoryAllocate(sizeof(Pixel) * raytraceImage.width * raytraceImage.height);
	raytraceImage.Fill(MakePixelFromRGBA(0, 0, 0, 255));

	raytracer.halfPixelSize.w = 0.5f / (f32)raytraceImage.width;
	raytracer.halfPixelSize.h = 0.5f / (f32)raytraceImage.height;

	//raytracer.rnd = RandomSeed(fplGetTimeInMillisecondsLP() % U64_MAX);
	raytracer.rnd = RandomSeed(1337);
	raytracer.settings.maxBounceCount = 4;
	raytracer.settings.raysPerPixelCount = 32;
}

static void InitApp(App &app, const u32 raytraceWidth, const u32 raytraceHeight) {
#if USE_OPENGL_NO_RAYTRACE
	InitGL();
#endif

	InitScene(app.scene);
	InitRaytracer(app.raytracer, raytraceWidth, raytraceHeight);
}

static void FillQueue(App &app, WorkQueue &queue, const TilingInfo &tilingInfo) {
	queue.Reset();

	fplAssert(queue.completionCount == 0);
	fplAssert(queue.workOrderCount == 0);
	fplAssert(queue.nextWorkOrderIndex == 0);

	u32 totalTileCount = tilingInfo.tileCountX * tilingInfo.tileCountY;

	for (u32 tileY = 0; tileY < tilingInfo.tileCountY; ++tileY) {
		for (u32 tileX = 0; tileX < tilingInfo.tileCountX; ++tileX) {
			u32 minX = tileX * tilingInfo.tileSizeX;
			u32 minY = tileY * tilingInfo.tileSizeY;
			u32 maxXPlusOne = fplMin(minX + tilingInfo.tileSizeX, tilingInfo.imageW);
			u32 maxYPlusOne = fplMin(minY + tilingInfo.tileSizeY, tilingInfo.imageH);
			queue.Push(&app.raytracer, &app.scene, minX, minY, maxXPlusOne, maxYPlusOne);
		}
	}

	fplAssert(queue.workOrderCount == totalTileCount);	
}

static void ReleaseApp(App &app) {
	fplMemoryFree(app.raytracer.image.pixels);
}

static bool RaytraceFromQueue(Worker *worker) {
	fplAssert(worker->queue != fpl_null);

	u32 orderIndex = 0;
	bool result = false;
	if (worker->queue->Pop(orderIndex)) {
		WorkOrder order = worker->queue->orders[orderIndex];
		result = RaytracePart(*worker, order);
		if (result) {
			fplAtomicIncrementU32(&worker->queue->completionCount);
		}
	}

	return(result);
}

static void WorkerThreadProc(const fplThreadHandle *thread, void *opaqueData) {
	Worker *worker = (Worker *)opaqueData;
	worker->Start();
	while (true) {
		if (worker->queue->IsEmpty() || worker->queue->IsFinished()) {
			fplMutexLock(&worker->lockMutex);
			fplConditionWait(&worker->nonEmptyCondition, &worker->lockMutex, FPL_TIMEOUT_INFINITE);
			fplMutexUnlock(&worker->lockMutex);
		}
		if (worker->IsStopped()) {
			break;
		}
		RaytraceFromQueue(worker);
	}
	worker->Stop();
}

int main(int argc, char **argv) {
	RandomSeries rnd = {};

	const u32 renderWidth = 1280;
	const u32 renderHeight = 720;

	const u32 raytraceWidth = renderWidth;
	const u32 raytraceHeight = renderHeight;

	fplSettings settings = fplMakeDefaultSettings();
	settings.window.windowSize.width = renderWidth;
	settings.window.windowSize.height = renderHeight;
	settings.window.isResizable = false;

#if USE_OPENGL_NO_RAYTRACE
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#else
	settings.video.driver = fplVideoDriverType_Software;
#endif

	if (fplPlatformInit(fplInitFlags_All, &settings)) {
#if USE_OPENGL_NO_RAYTRACE
		if (!fglLoadOpenGL(true)) {
			fplPlatformRelease();
			return -1;
		}
#endif

		// Fixed backbuffer
		fplResizeVideoBackBuffer(renderWidth, renderHeight);

		// Init worker parameters
		TilingInfo tilingInfo = {};
		tilingInfo.imageW = raytraceWidth;
		tilingInfo.imageH = raytraceHeight;
		tilingInfo.tileSizeX = tilingInfo.tileSizeY = 64;
		tilingInfo.tileCountX = (raytraceWidth / tilingInfo.tileSizeX) + 1;
		tilingInfo.tileCountY = (raytraceHeight / tilingInfo.tileSizeY) + 1;

		// @NOTE(final): We use the STL to make our life easier, so we need to placement-new-initialize our App structure
		App app = {};
		new(&app)App();
		InitApp(app, raytraceWidth, raytraceHeight);

		// Queue
		u32 maxTileCount = tilingInfo.tileCountX * tilingInfo.tileCountY;
		WorkQueue queue = {};
		queue.Init(maxTileCount);

		// Init worker
		u32 cpuCoreCount = (u32)fplGetProcessorCoreCount();
		fplAssert(cpuCoreCount > 0);
		u32 workerCount = (u32)cpuCoreCount - 1;
		Worker *workers = new Worker[workerCount];
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplClearStruct(worker);
			fplMutexInit(&worker->lockMutex);
			fplConditionInit(&worker->nonEmptyCondition);
			worker->queue = &queue;
			worker->thread = fplThreadCreate(WorkerThreadProc, worker);
		}

		bool refresh = true;
		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {
				switch (ev.type) {
					case fplEventType::fplEventType_Keyboard:
					{
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release &&
								ev.keyboard.mappedKey == fplKey_Space) {
								refresh = true;
							}
						}
					} break;
				}
			}

			if (refresh) {
				refresh = false;
				if (queue.IsEmpty() || queue.IsFinished()) {
					FillQueue(app, queue, tilingInfo);

					for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
						Worker *worker = workers + workerIndex;
						fplConditionSignal(&worker->nonEmptyCondition);
					}
				}
			}

			Render(app);

			fplVideoFlip();
		}

		// Send stop signal to all workers
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplAtomicStoreS32((volatile s32 *)&worker->state, (s32)WorkerState::Stopped);
			fplConditionSignal(&worker->nonEmptyCondition);
		}

		// Wait for all threads to finish
		fplThreadWaitForAll(&workers[0].thread, workerCount, sizeof(Worker), FPL_TIMEOUT_INFINITE);

		// Terminate unfinished threads
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplThreadTerminate(worker->thread);
		}

		// Release worker resources
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplConditionDestroy(&worker->nonEmptyCondition);
			fplMutexDestroy(&worker->lockMutex);
		}
		delete[] workers;

		queue.Release();

		ReleaseApp(app);

#if USE_OPENGL_NO_RAYTRACE
		fglUnloadOpenGL();
#endif
		fplPlatformRelease();
		return 0;
	}
}