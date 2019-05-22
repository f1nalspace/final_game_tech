// This is incomplete demo!

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <final_math.h>
#include <final_geometry.h>

#include <vector>
#include <new>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static Vec3f UnitRight = V3f(1, 0, 0);
static Vec3f UnitUp = V3f(0, 1, 0);
static Vec3f UnitForward = V3f(0, 0, 1);

enum class ObjectKind : int32_t {
	None = 0,
	Plane,
	Sphere,
};

struct Object {
	union {
		Plane3f plane;
		Sphere3f sphere;
	};
	ObjectKind kind;
};

struct Camera {
	Vec3f eye;
	Vec3f target;
	Vec3f up;
	float fov;
	float zNear;
	float zFar;
};

struct Scene {
	Camera camera;
	std::vector<Object> objects;
	void AddPlane(const Vec3f &normal, const float distance) {
		Object obj = {};
		obj.kind = ObjectKind::Plane;
		obj.plane.normal = normal;
		obj.plane.distance = distance;
		objects.push_back(obj);
	}
	void AddSphere(const Vec3f &origin, const float radius) {
		Object obj = {};
		obj.kind = ObjectKind::Sphere;
		obj.sphere.origin = origin;
		obj.sphere.radius = radius;
		objects.push_back(obj);
	}
};

struct Image32Data {
	uint8_t *pixels;
	uint32_t width;
	uint32_t height;
};

struct Renderer {
	GLuint textureId;
	GLuint imageBufferId;
};

struct App {
	Scene scene;
	Renderer renderer;
	Image32Data image;
};

static void DrawSphere(const Vec3f &origin, float radius, int steps = 20) {
	float x, y, z;
	for (float alpha = 0.0; alpha < Pi32; alpha += Pi32 / steps) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float beta = 0.0; beta < 2.01 * Pi32; beta += Pi32 / steps) {
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

static void DrawPlane(const Vec3f &normal, const float distance, float infiniteSize) {
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

static void DrawCube(const Vec3f &pos, const float radius) {
	static float cubeVertices[] =
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
			const float *v = &cubeVertices[side * 6 * 3 + vertexIndex * 3];
			float x = pos.x + v[0] * radius;
			float y = pos.y + v[1] * radius;
			float z = pos.z + v[2] * radius;
			glVertex3f(x, y, z);
		}
	}
	glEnd();
}

static void Init(App &app) {
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

	// App
	app.image.width = 1080;
	app.image.height = 720;
	app.image.pixels = (uint8_t *)malloc(4 * app.image.width * app.image.height);

	// Renderer
	glGenBuffers(1, &app.renderer.imageBufferId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app.renderer.imageBufferId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


	// Scene
	app.scene.camera.eye = V3f(0, 4, 10);
	app.scene.camera.target = V3f(0, 0, 0);
	app.scene.camera.up = UnitUp;
	app.scene.camera.fov = 45.0f;
	app.scene.camera.zNear = 0.5f;
	app.scene.camera.zFar = 100.0f;

	app.scene.AddPlane(V3f(0, 1, 0), 0.0f);
	app.scene.AddSphere(V3f(-4, 0, 0), 1);
	app.scene.AddSphere(V3f(7, 2, 0), 2);
	app.scene.AddSphere(V3f(0, 1, -6), 1.5f);
	app.scene.AddSphere(V3f(0.5f, 1, 6), 0.75f);
}

static void Release(App &app) {
	glDeleteBuffers(1, &app.renderer.imageBufferId);
	free(app.image.pixels);
}

static void Render(const Scene &scene) {
	fplWindowSize size = {};
	fplGetWindowSize(&size);
	glViewport(0, 0, size.width, size.height);

	const float aspect = size.height > 0 ? size.width / (float)size.height : 1.0f;
	const bool wireframe = true;

	Vec3f camEye = scene.camera.eye;
	Vec3f camTarget = scene.camera.target;
	Vec3f camUp = scene.camera.up;
	float fov = scene.camera.fov;
	float zNear = scene.camera.zNear;
	float zFar = scene.camera.zFar;

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
	const float infinityPlaneSize = 100.0f;
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
	float crossLen = 3.0f;
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
}

enum class WorkerState : int32_t {
	Stopped = 0,
	Running,
};

struct WorkerData {
	volatile WorkerState state;
	const Scene *scene;
	Image32Data *image;
};

static void WorkerThread(const fplThreadHandle *thread, void *opaqueData) {
	WorkerData *data = (WorkerData *)opaqueData;
	fplAtomicExchangeS32((volatile int32_t *)&data->state, (int32_t)WorkerState::Running);
	while (true) {
		WorkerState newState = (WorkerState)fplAtomicLoadS32((volatile int32_t *)&data->state);
		if (newState == WorkerState::Stopped) {
			break;
		}

		// @TODO(final): Ray tracing here!
	}
	fplAtomicExchangeS32((volatile int32_t *)&data->state, (int32_t)WorkerState::Stopped);
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {

			App app = {};
			new(&app)App();

			Scene *scene = &app.scene;
			Image32Data *image = &app.image;

			Init(app);

			WorkerData data = {};
			data.scene = scene;
			data.image = image;
			fplThreadHandle *thread = fplThreadCreate(WorkerThread, &data);

			while (fplWindowUpdate()) {
				fplPollEvents();
				Render(*scene);
				fplVideoFlip();
			}

			fplAtomicExchangeS32((volatile int32_t *)&data.state, (int32_t)WorkerState::Stopped);
			fplThreadWaitForOne(thread, FPL_TIMEOUT_INFINITE);
			fplThreadTerminate(thread);

			Release(app);

			fglUnloadOpenGL();
		}
		fplPlatformRelease();
	}
	return 0;
}