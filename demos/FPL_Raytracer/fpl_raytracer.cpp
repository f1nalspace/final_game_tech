/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Raytracer

Description:
	Very simple 3D Raytracer demo, using the simplest possible way to do raytracing.
	Right know, its incomplete and does nothing more than producing pixel values for the very first hit.
	There is no bouncing going on or any kind of fancy lighting equations.

	* Inspired by handmade ray (Casey Muratori)

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
typedef int32_t s32;
typedef float f32;

constexpr u8 U8_MAX = UCHAR_MAX;
constexpr u32 U32_MAX = UINT32_MAX;
constexpr s32 S32_MAX = INT32_MAX;
constexpr f32 F32_MAX = FLT_MAX;

#if USE_OPENGL_NO_RAYTRACE
#	define FGL_IMPLEMENTATION
#	include <final_dynamic_opengl.h>

#	include <glm/glm.hpp>
#	include <glm/gtc/matrix_transform.hpp>
#endif

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
	RandomSeries rnd;

	Camera camera;

	std::vector<Object> objects;
	std::vector<Material> materials;

	u32 AddMaterial(const Vec3f &emitColor, const Vec3f &reflectColor) {
		fplAssert(materials.size() < (U32_MAX - 1));
		u32 result = (u32)materials.size();
		Material mat = {};
		mat.emitColor = emitColor;
		mat.reflectColor = reflectColor;
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

struct Renderer {
};

struct App {
	Scene scene;
	Renderer renderer;
	Image32 raytraceImage;
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
	} else {
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
#else
	fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();
	Image32 raytraceImage = app.raytraceImage;
	u32 sourceLineWidth = raytraceImage.width * 4;

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
#endif
}

enum class WorkerState : s32 {
	Stopped = 0,
	Running,
};

struct WorkerData {
	volatile WorkerState state;
	Scene *scene;
	Image32 *image;
	bool updateImage;

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

static Vec3f RayCast(const Scene &scene, const Ray3f &initialRay, const WorkerData &data) {
	fplAssert(scene.materials.size() > 0);
	const Material &defaultMat = scene.materials[0];

	const u32 rayCount = 8;
	const f32 tolerance = 1e-6f;
	f32 minHitDistance = 0.0f;

	Ray3f ray = MakeRay(initialRay.origin, initialRay.direction);

	Vec3f result = V3f(0, 0, 0);
	Vec3f attenuation = V3f(1, 1, 1);

	for (u32 rayIndex = 0; rayIndex < rayCount; ++rayIndex) {
		Vec3f hitPoint = V3f();
		Vec3f hitNormal = V3f();
		const Object *hitObject = fpl_null;
		u32 hitMaterialIndex = 0;
		f32 nearestDistance = F32_MAX;

		for (u32 objectIndex = 0, objectCount = (u32)scene.objects.size(); objectIndex < objectCount; ++objectIndex) {
			if (data.IsStopped())
				break;

			const Object *obj = &scene.objects[objectIndex];

			f32 t = -F32_MAX;
			bool isHit = false;
			if (obj->kind == ObjectKind::Plane) {
				if (RayPlaneIntersection(ray, obj->plane, t, tolerance)) {
					if ((t > minHitDistance) && (t < nearestDistance)) {
						nearestDistance = t;
						hitMaterialIndex = obj->materialIndex;
						hitObject = obj;

						hitPoint = t * ray.direction + ray.origin;
						hitNormal = obj->plane.normal;
					}
				}
			} else if (obj->kind == ObjectKind::Sphere) {
				if (RaySphereIntersection(ray, obj->sphere, t, tolerance)) {
					if ((t > minHitDistance) && (t < nearestDistance)) {
						nearestDistance = t;
						hitMaterialIndex = obj->materialIndex;
						hitObject = obj;

						hitPoint = t * ray.direction + ray.origin;
						hitNormal = Vec3Normalize(hitPoint - obj->sphere.origin);
					}
				}
			}
		}

		if (hitMaterialIndex) {
			fplAssert(hitMaterialIndex < scene.materials.size());
			const Material &hitMaterial = scene.materials[hitMaterialIndex];

			result += Vec3Hadamard(attenuation, hitMaterial.emitColor);
			attenuation = Vec3Hadamard(attenuation, hitMaterial.reflectColor);

			ray.origin = hitPoint;
			ray.direction = hitNormal;
		} else {
			result += Vec3Hadamard(attenuation, defaultMat.emitColor);
			break;
		}
	}

	return(result);
}

static void RaytraceImage(Scene &scene, Image32 &outputImage, const WorkerData &data) {
#if 0
	const Vec3f cameraPosition = V3f(0, -10, 1);
	const Vec3f cameraUp = V3f(0, 0, 1);
	const Vec3f cameraTarget = V3f(0, 0, 0);
#else
	const Vec3f cameraPosition = scene.camera.eye;
	const Vec3f cameraUp = scene.camera.up;
	const Vec3f cameraTarget = scene.camera.target;
#endif

	Vec3f cameraZ = Vec3Normalize(cameraPosition - cameraTarget);
	Vec3f cameraX = Vec3Normalize(Vec3Cross(cameraUp, cameraZ));
	Vec3f cameraY = Vec3Normalize(Vec3Cross(cameraZ, cameraX));

	f32 filmWidth = 1.0f;
	f32 filmHeight = 1.0f;

	// Aspect ratio correction
	if (outputImage.width > outputImage.height) {
		filmHeight = filmWidth * ((f32)outputImage.height / (f32)outputImage.width);
	} else if (outputImage.height > outputImage.width) {
		filmWidth = filmHeight * ((f32)outputImage.width / (f32)outputImage.height);
	}

	f32 filmHalfWidth = 0.5f * filmWidth;
	f32 filmHalfHeight = 0.5f * filmHeight;

	f32 filmDistance = 1.0f;
	Vec3f filmCenter = cameraPosition - filmDistance * cameraZ;

	for (u32 y = 0; y < outputImage.height; ++y) {
		Pixel *row = outputImage.pixels + (y * outputImage.width);

		f32 ratioY = (f32)y / (f32)outputImage.height;
		f32 filmY = -1.0f + 2.0f * ratioY;

		for (u32 x = 0; x < outputImage.width; ++x) {
			if (data.IsStopped())
				break;

			f32 ratioX = (f32)x / (f32)outputImage.width;
			f32 filmX = -1.0f + 2.0f * ratioX;

			Vec3f filmP = filmCenter + filmX * filmHalfWidth * cameraX + filmY * filmHalfHeight * cameraY;

			Vec3f rayOrigin = cameraPosition;
			Vec3f rayDirection = Vec3Normalize(filmP - cameraPosition);
			Ray3f ray = MakeRay(rayOrigin, rayDirection);

			Vec3f color = RayCast(scene, ray, data);

			Vec4f finalColor = V4f(color, 1.0f);

			Pixel outputPixel = PixelPack(finalColor);
			*row = outputPixel;
			++row;
		}

		if (data.IsStopped())
			break;
	}
}

static void WorkerThreadProc(const fplThreadHandle *thread, void *opaqueData) {
	WorkerData *data = (WorkerData *)opaqueData;
	data->Start();
	while (true) {
		if (data->IsStopped()) {
			break;
		}
		if (data->updateImage) {
			data->updateImage = false;
			RaytraceImage(*data->scene, *data->image, *data);
		}
	}
	data->Stop();

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

static void Init(App &app, const u32 renderWidth, const u32 rendereHeight) {
#if USE_OPENGL_NO_RAYTRACE
	InitGL();
#endif

	// App
	const fplSettings *settings = fplGetCurrentSettings();
	app.raytraceImage.width = renderWidth;
	app.raytraceImage.height = rendereHeight;
	app.raytraceImage.pixels = (Pixel *)calloc(1, sizeof(Pixel) * app.raytraceImage.width * app.raytraceImage.height);
	app.raytraceImage.Fill(MakePixel(0, 0, 0, 255));

	// Scene

	//app.scene.rnd.index = fplGetTimeInMillisecondsLP() % U32_MAX;
	app.scene.rnd.index = 1337;

	app.scene.camera.eye = V3f(0, -10, 1);
	app.scene.camera.target = V3f(0, 0, 0);
	app.scene.camera.up = UnitUp;
	app.scene.camera.fov = 45.0f;
	app.scene.camera.zNear = 0.5f;
	app.scene.camera.zFar = 100.0f;

	app.scene.AddMaterial(V3f(0.3f, 0.4f, 0.5f), {});

	u32 redMat = app.scene.AddMaterial({}, V3f(0.5f, 0.5f, 0.5f));
	u32 blueMat = app.scene.AddMaterial({}, V3f(0.7f, 0.5f, 0.3f));

	app.scene.AddPlane(V3f(0, 0, 1), 0.0f, redMat);
	app.scene.AddSphere(V3f(0, 0, 0), 1.0f, blueMat);

#if 0
	app.scene.AddPlane(V3f(0, 1, 0), 0.0f);
	app.scene.AddSphere(V3f(-4, 0, 0), 1);
	app.scene.AddSphere(V3f(7, 2, 0), 2);
	app.scene.AddSphere(V3f(0, 1, -6), 1.5f);
	app.scene.AddSphere(V3f(0.5f, 1, 6), 0.75f);
#endif
}

static void Release(App &app) {
	free(app.raytraceImage.pixels);
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

		App app = {};

		// @NOTE(final): We use the STL to make our life easier, so we need to placement-new-initialize our App structure
		new(&app)App();

		Init(app, raytraceWidth, raytraceHeight);

		WorkerData data = {};
		data.scene = &app.scene;
		data.image = &app.raytraceImage;
		data.updateImage = true;
		fplThreadHandle *workerThread = fplThreadCreate(WorkerThreadProc, &data);

		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {
				switch (ev.type) {
					case fplEventType::fplEventType_Keyboard:
					{
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release && ev.keyboard.mappedKey == fplKey_Space) {
								data.updateImage = true;
							}
						}
					} break;
				}
			}
			Render(app);
			fplVideoFlip();
		}

		fplAtomicExchangeS32((volatile s32 *)&data.state, (s32)WorkerState::Stopped);
		fplThreadWaitForOne(workerThread, FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(workerThread);

		Release(app);

#if USE_OPENGL_NO_RAYTRACE
		fglUnloadOpenGL();
#endif
		fplPlatformRelease();
		return 0;
	}
}