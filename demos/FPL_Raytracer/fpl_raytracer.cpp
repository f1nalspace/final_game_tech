/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Raytracer

Description:
	Very simple multi-threaded 3D Raytracer demo.
	Right know, we dont do any physical correct lighting whatsoever.

	* Inspired by handmade ray (Casey Muratori)

Todo:
	- Fix camera perspective (identical to opengl)
	- Fix wrong random bounce and do a proper random distribution
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
	## 2019-06-01
	- Initial version
-------------------------------------------------------------------------------
*/

// Define this to enable opengl preview, instead of doing the actual raytracing
#define USE_OPENGL_NO_RAYTRACE 0

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

// Custom types to save a bit of typing :D
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef float f32;

constexpr u8 U8_MAX = UCHAR_MAX;
constexpr u32 U32_MAX = UINT32_MAX;
constexpr s32 S32_MAX = INT32_MAX;
constexpr f32 F32_MAX = FLT_MAX;

#if USE_OPENGL_NO_RAYTRACE
#	define FGL_IMPLEMENTATION
#	include <final_dynamic_opengl.h>

#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <final_math.h>
#include <final_geometry.h>

#include <vector>
#include <new>

struct Image32 {
	Pixel *pixels;
	u32 width;
	u32 height;

	inline void Fill(const Pixel &color) {
		for (size_t i = 0; i < width * height; ++i)
			pixels[i] = color;
	}
};

struct RandomSeries {
	uint32_t index;
};

inline uint32_t RandomU32(RandomSeries *series) {
	// https://de.wikipedia.org/wiki/Xorshift
	series->index ^= (series->index << 13);
	series->index ^= (series->index >> 17);
	series->index ^= (series->index << 5);
	return (series->index);
}

inline uint8_t RandomU8(RandomSeries *series) {
	uint8_t result = RandomU32(series) % U8_MAX;
	return(result);
}

// -1.0 to +1.0
inline f32 RandomBilateral(RandomSeries *series) {
	s32 s = RandomU32(series) % S32_MAX;
	f32 result = s / (f32)S32_MAX;
	fplAssert(result >= -1.0f && result <= 1.0f);
	return(result);
}

// 0.0 to 1.0
inline f32 RandomUnilateral(RandomSeries *series) {
	u32 u = RandomU32(series);
	f32 result = u / (f32)U32_MAX;
	fplAssert(result >= 0.0f && result <= 1.0f);
	return(result);
}

static Vec3f UnitRight = V3f(1, 0, 0);
static Vec3f UnitUp = V3f(0, 0, 1);
static Vec3f UnitForward = V3f(0, 1, 0);

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
	Vec3f u = Vec3Normalize(Vec3Cross(normal, UnitRight));
	Vec3f v = Vec3Normalize(Vec3Cross(normal, u));
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

	const f32 aspect = size.height > 0 ? size.width / (f32)size.height : 1.0f;
	const bool wireframe = true;

	const Scene &scene = app.scene;

	Vec3f camEye = scene.camera.eye;
	Vec3f camTarget = scene.camera.target;
	Vec3f camUp = scene.camera.up;

	f32 fov = scene.camera.fov;
	f32 zNear = scene.camera.zNear;
	f32 zFar = scene.camera.zFar;

#if USE_OPENGL_NO_RAYTRACE
	glViewport(0, 0, size.width, size.height);

#if 1
	Mat4f projMat = Mat4PerspectiveRH(fov, aspect, zNear, zFar);
	Mat4f viewMat = Mat4LookAtRH(camEye, camTarget, camUp);
	Mat4f viewProjMat = projMat * viewMat;
	glLoadMatrixf(&viewProjMat.m[0]);
#else
	glm::mat4 projMat = glm::perspective(fov, aspect, zNear, zFar);
	glm::mat4 viewMat = glm::lookAt(glm::vec3(camEye.x, camEye.y, camEye.z), glm::vec3(camTarget.x, camTarget.y, camTarget.z), glm::vec3(camUp.x, camUp.y, camUp.z));
	glm::mat4 viewProjMat = projMat * viewMat;
	glLoadMatrixf(&viewProjMat[0][0]);
#endif

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

#if 1
	const f32 infinityPlaneSize = 100.0f;
	for (const Object &obj : scene.objects) {
		switch (obj.kind) {
		case ObjectKind::Plane:
			glColor3f(0, 0, 1);
			DrawPlane(obj.plane.normal, obj.plane.distance, infinityPlaneSize);
			break;
		case ObjectKind::Sphere:
			glColor3f(1, 0, 0);
			DrawSphere(obj.sphere.origin, obj.sphere.radius);
			break;
		}
	}
#endif

#if 1
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

	Vec3f normal = Vec3Normalize(camTarget - camEye);

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
			uint32_t color = BGRA8(sourcePixel);
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

struct Worker;

struct WorkOrder {
	const Worker *worker;
	const Scene *scene;
	Raytracer *raytracer;
	u32 xMin;
	u32 yMin;
	u32 xMaxPlusOne;
	u32 yMaxPlusOne;
};

struct WorkQueue {
	WorkOrder *orders;
	u8 padding1[8];

	u32 capacity;
	volatile u32 nextWorkOrderIndex;
	volatile u32 completionCount;
	volatile u32 workOrderCount;

	bool IsEmpty() {
		bool result = workOrderCount == 0;
		return(result);
	}

	bool IsFinished() {
		bool result = (workOrderCount == completionCount);
		return(result);
	}

	void Init(const u32 capacity) {
		orders = (WorkOrder *)calloc(capacity, sizeof(*orders));
		this->capacity = capacity;
		fplAtomicExchangeU32(&workOrderCount, 0);
		fplAtomicExchangeU32(&nextWorkOrderIndex, 0);
		fplAtomicExchangeU32(&completionCount, 0);
	}

	void Release() {
		free(orders);
	}

	void Reset() {
		fplAtomicExchangeU32(&workOrderCount, 0);
		fplAtomicExchangeU32(&nextWorkOrderIndex, 0);
		fplAtomicExchangeU32(&completionCount, 0);
	}

	void Push(Raytracer *rayTracer, const Scene *scene, const u32 xMin, const u32 yMin, const u32 xMaxPlusOne, const u32 yMaxPlusOne) {
		fplAssert(workOrderCount < capacity);
		u32 index = fplAtomicFetchAndAddU32(&workOrderCount, 1);
		WorkOrder *order = orders + index;
		*order = {};
		order->raytracer = rayTracer;
		order->scene = scene;
		order->worker = fpl_null;
		order->xMin = xMin;
		order->yMin = yMin;
		order->xMaxPlusOne = xMaxPlusOne;
		order->yMaxPlusOne = yMaxPlusOne;
	}

	bool Pop(WorkOrder &order) {
		if (workOrderCount == 0) {
			return(false);
		}

		u32 index = fplAtomicFetchAndAddU32(&nextWorkOrderIndex, 1);
		if (!(index < workOrderCount)) {
			return(false);
		}
		order = orders[index];
		orders[index] = {};
		return(true);
	}
};

struct Worker {
	WorkQueue *queue;
	fplThreadHandle *thread;
	volatile WorkerState state;

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
	f32 denom = Vec3Dot(plane.normal, ray.direction);
	if ((denom < -tolerance) || (denom > tolerance)) {
		f32 t = (-plane.distance - Vec3Dot(plane.normal, ray.origin)) / denom;
		out = t;
		return(true);
	}
	return(false);
}

static bool RaySphereIntersection(const Ray3f &ray, const Sphere3f &sphere, f32 &out, const float tolerance) {
	Vec3f rayRelativeOrigin = ray.origin - sphere.origin;
	f32 a = Vec3Dot(ray.direction, ray.direction);
	f32 b = 2.0f * Vec3Dot(ray.direction, rayRelativeOrigin);
	f32 c = Vec3Dot(rayRelativeOrigin, rayRelativeOrigin) - sphere.radius * sphere.radius;

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

static bool RaytracePart(WorkOrder &order) {
	const Worker *worker = order.worker;
	const Scene &scene = *order.scene;
	Raytracer &raytracer = *order.raytracer;
	Image32 &image = raytracer.image;

	f32 fov = DegreesToRadians(25.0f);
	f32 aspect = (f32)image.width / (float)image.height;

	const Vec3f cameraPosition = scene.camera.eye;
	const Vec3f cameraUp = scene.camera.up;
	const Vec3f cameraTarget = scene.camera.target;

	const Vec3f cameraZ = Vec3Normalize(cameraPosition - cameraTarget);
	const Vec3f cameraX = Vec3Normalize(Vec3Cross(cameraUp, cameraZ));
	const Vec3f cameraY = Vec3Normalize(Vec3Cross(cameraZ, cameraX));

	const Vec2f halfPixelSize = raytracer.halfPixelSize;

	f32 filmDistance = 1.0f;
	Vec3f filmCenter = cameraPosition - filmDistance * cameraZ;

	u32 raysPerPixel = raytracer.settings.raysPerPixelCount;
	u32 maxBounceCount = raytracer.settings.maxBounceCount;

	f32 contrib = 1.0f / (f32)raysPerPixel;

	fplAssert(scene.materials.size() > 0);
	const Material &defaultMaterial = scene.materials[0];

	for (u32 y = order.yMin; y < order.yMaxPlusOne; ++y) {
		Pixel *row = image.pixels + (y * image.width);

		f32 ratioY = (f32)y / (f32)image.height;
		f32 filmY = -1.0f + 2.0f * ratioY;

		Pixel *col = row + order.xMin;
		for (u32 x = order.xMin; x < order.xMaxPlusOne; ++x) {
			if (worker->IsStopped())
				return(false);

			f32 ratioX = (f32)x / (f32)image.width;
			f32 filmX = -1.0f + 2.0f * ratioX;

			Vec3f finalColor = {};

			for (u32 rayIndex = 0; rayIndex < raysPerPixel; ++rayIndex) {
				if (worker->IsStopped())
					return(false);

				f32 offsetX = RandomBilateral(&raytracer.rnd) * halfPixelSize.w;
				f32 offsetY = RandomBilateral(&raytracer.rnd) * halfPixelSize.h;

				f32 perspectiveX = (filmX + offsetX) * Tan(fov * 0.5f) * aspect;
				f32 perspectiveY = (filmY + offsetY) * Tan(fov * 0.5f);

				Vec3f filmP = filmCenter + (perspectiveX * cameraX) + (perspectiveY * cameraY);

				Vec3f rayOrigin = cameraPosition;
				Vec3f rayDirection = Vec3Normalize(filmP - cameraPosition);

				Ray3f ray = MakeRay(rayOrigin, rayDirection);

				const f32 tolerance = 1e-6f;
				f32 minHitDistance = 0.0f;

				Vec3f sample = {};
				Vec3f attenuation = V3f(1, 1, 1);

				for (u32 bounceIndex = 0; bounceIndex < maxBounceCount; ++bounceIndex) {
					if (worker->IsStopped())
						return(false);

					f32 hitDistance = F32_MAX;
					u32 hitMaterialIndex = 0;
					Vec3f hitNormal = V3f();

					for (u32 objectIndex = 0, objectCount = (u32)scene.objects.size(); objectIndex < objectCount; ++objectIndex) {
						if (worker->IsStopped())
							return(false);

						const Object *obj = &scene.objects[objectIndex];

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
						}
						else if (obj->kind == ObjectKind::Sphere) {
							if (RaySphereIntersection(ray, obj->sphere, t, tolerance)) {
								if ((t > minHitDistance) && (t < hitDistance)) {
									hitDistance = t;
									hitMaterialIndex = obj->materialIndex;
									Vec3f relativeOrigin = ray.origin - obj->sphere.origin;
									hitNormal = Vec3Normalize(t * ray.direction + relativeOrigin);
								}
							}
						}
					}

					if (hitMaterialIndex) {
						fplAssert(hitMaterialIndex < scene.materials.size());
						const Material &hitMaterial = scene.materials[hitMaterialIndex];

						sample += Vec3Hadamard(attenuation, hitMaterial.emitColor);

						f32 cosineAttenuation = Vec3Dot(-ray.direction, hitNormal);
						if (cosineAttenuation < 0) {
							cosineAttenuation = 0;
						}
						attenuation = Vec3Hadamard(attenuation, cosineAttenuation * hitMaterial.reflectColor);

						Vec3f pureBounce = ray.direction - 2.0f * Vec3Dot(ray.direction, hitNormal) * hitNormal;

						// @TODO(final): This is NOT a proper way to produce a random bounce. Do proper distribution based bounce
						Vec3f randomBounce = Vec3Normalize(hitNormal + V3f(RandomBilateral(&raytracer.rnd), RandomBilateral(&raytracer.rnd), RandomBilateral(&raytracer.rnd)));

						// Ray for next bounce
						ray.origin += hitDistance * ray.direction;
						ray.direction = Vec3Normalize(Vec3Lerp(randomBounce, hitMaterial.scatter, pureBounce));
					}
					else {
						sample += Vec3Hadamard(attenuation, defaultMaterial.emitColor);
						break;
					}
				}

				finalColor += contrib * sample;
			}

			Pixel outputPixel = LinearToPixel(V4f(finalColor, 1.0f));
			*col = outputPixel;
			++col;
		}

		++row;

		if (worker->IsStopped())
			return(false);
	}

	return(true);
}

static bool RaytraceFromQueue(Worker *worker) {
	fplAssert(worker->queue != fpl_null);

	WorkOrder order = {};
	if (!worker->queue->Pop(order)) {
		return(false);
	}

	order.worker = worker;

	bool result = RaytracePart(order);
	if (result) {
		fplAtomicFetchAndAddU32(&worker->queue->completionCount, 1);
	}

	return(result);
}

static void WorkerThreadProc(const fplThreadHandle *thread, void *opaqueData) {
	Worker *worker = (Worker *)opaqueData;
	worker->Start();
	while (true) {
		if (worker->IsStopped()) {
			break;
		}
		if (!RaytraceFromQueue(worker)) {
			fplThreadYield();
		}
	}
	worker->Stop();
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
	scene.camera.eye = V3f(0, -10, 1);
	scene.camera.target = V3f(0, 0, 0);
	scene.camera.up = UnitUp;
	scene.camera.fov = 45.0f;
	scene.camera.zNear = 0.5f;
	scene.camera.zFar = 100.0f;

	scene.AddMaterial(V3f(0.152f, 0.22745f, 0.3647f), {});

	u32 floorMat = scene.AddMaterial(V3f(0, 0.0f, 0), V3f(0.1f, 0.5f, 0.1f), 0.75f);
	u32 whiteMat = scene.AddMaterial(V3f(0.0f, 0.0f, 0.0f), V3f(1.0f, 1.0f, 1.0f), 1.0f);
	u32 redMat = scene.AddMaterial(V3f(0.25f, 0.0f, 0.0f), V3f(1.0f, 0.0f, 0.0f), 1.0f);
	u32 blueMat = scene.AddMaterial(V3f(0.0f, 0.0f, 0.25f), V3f(0.0f, 0.0f, 6.0f), 1.0f);

	scene.AddPlane(V3f(0, 0, 1), 0.0f, floorMat);
	scene.AddSphere(V3f(0, 0, 0), 1.0f, whiteMat);
	scene.AddSphere(V3f(1, -2, 0.3f), 0.5f, redMat);
	scene.AddSphere(V3f(-1.0f, -0.5f, 0.9f), 0.3f, blueMat);
}

static void InitRaytracer(Raytracer &raytracer, const u32 raytraceWidth, const u32 raytraceHeight) {
	Image32 &raytraceImage = raytracer.image;
	raytraceImage.width = raytraceWidth;
	raytraceImage.height = raytraceHeight;
	raytraceImage.pixels = (Pixel *)calloc(1, sizeof(Pixel) * raytraceImage.width * raytraceImage.height);
	raytraceImage.Fill(MakePixel(0, 0, 0, 255));

	raytracer.halfPixelSize.w = 0.5f / (f32)raytraceImage.width;
	raytracer.halfPixelSize.h = 0.5f / (f32)raytraceImage.height;

	//raytracer.rnd.index = fplGetTimeInMillisecondsLP() % U32_MAX;
	raytracer.rnd.index = 1337;
	raytracer.settings.maxBounceCount = 8;
	raytracer.settings.raysPerPixelCount = 128;
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
	free(app.raytracer.image.pixels);
}

int main(int argc, char **argv) {
	RandomSeries rnd = {};

	float blubb = RandomUnilateral(&rnd);

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
		tilingInfo.tileSizeX = tilingInfo.tileSizeY = 128;
		tilingInfo.tileCountX = (raytraceWidth / tilingInfo.tileSizeX) + 1;
		tilingInfo.tileCountY = (raytraceHeight / tilingInfo.tileSizeY) + 1;

		// Queue
		u32 maxTileCount = tilingInfo.tileCountX * tilingInfo.tileCountY;
		WorkQueue queue = {};
		queue.Init(maxTileCount);

		// Init worker
		size_t cpuCoreCount = fplGetProcessorCoreCount();
		size_t workerCount = cpuCoreCount - 1;
		Worker *workers = new Worker[workerCount];
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			worker->queue = &queue;
			worker->thread = fplThreadCreate(WorkerThreadProc, worker);
		}

		// @NOTE(final): We use the STL to make our life easier, so we need to placement-new-initialize our App structure
		App app = {};
		new(&app)App();
		InitApp(app, raytraceWidth, raytraceHeight);

		bool refresh = true;
		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {
				switch (ev.type) {
				case fplEventType::fplEventType_Keyboard:
				{
					if (ev.keyboard.type == fplKeyboardEventType_Button) {
						if (ev.keyboard.buttonState == fplButtonState_Release && ev.keyboard.mappedKey == fplKey_Space) {
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
				}
			}

			Render(app);
			fplVideoFlip();
		}

		// Send stop signal to all workers
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplAtomicExchangeS32((volatile s32 *)&worker->state, (s32)WorkerState::Stopped);
		}

		// Wait for all threads to finish
		fplThreadWaitForAll(&workers[0].thread, workerCount, sizeof(Worker), FPL_TIMEOUT_INFINITE);

		// Terminate unfinished threads
		for (u32 workerIndex = 0; workerIndex < workerCount; ++workerIndex) {
			Worker *worker = workers + workerIndex;
			fplThreadTerminate(worker->thread);
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