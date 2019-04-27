#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <final_math.h>

#include <vector>
#include <new>

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

struct Scene {
	std::vector<Object> objects;
	void AddPlane(const Vec3f &origin, const Vec3f &normal) {
		Object obj = {};
		obj.kind = ObjectKind::Plane;
		obj.plane.origin = origin;
		obj.plane.normal = normal;
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

static void Init(Scene &scene) {
	glMatrixMode(GL_MODELVIEW);

	// Left-handed coordinate system
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Cull back faces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_CCW);

	glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
	scene.AddSphere(V3f(-5, 10, 1), 1);
}

static void DrawSphere(const Vec3f &origin, float radius, int steps = 20) {
	for (float alpha = 0.0; alpha < Pi32; alpha += Pi32 / steps) {
		glBegin(GL_TRIANGLE_STRIP);
		for (float beta = 0.0; beta < 2.01 * Pi32; beta += Pi32 / steps) {
			float x = radius * cos(beta)*sin(alpha);
			float y = radius * sin(beta)*sin(alpha);
			float z = radius * cos(alpha);
			glVertex3f(x, y, z);
			x = radius * cos(beta)*sin(alpha + Pi32 / steps);
			y = radius * sin(beta)*sin(alpha + Pi32 / steps);
			z = radius * cos(alpha + Pi32 / steps);
			glVertex3f(x, y, z);
		}
		glEnd();
	}
}

static void Render(const Scene &scene) {
	fplWindowSize size = {};
	fplGetWindowSize(&size);
	float aspect = size.width / (float)size.height;

	glViewport(0, 0, size.width, size.height);

	Vec3f center = V3f(0, 0, 0);
	Vec3f eye = V3f(0, 0, 10);
	Vec3f up = V3f(0, 1, 0);

	Mat4f projMat = Mat4PerspectiveLH(30.0, aspect, 0.5, 100.0);
	Mat4f viewMat = Mat4LookAtLH(eye, center, up);
	Mat4f viewProjMat = projMat * viewMat;
	glLoadMatrixf(&viewProjMat.m[0]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const Object &obj : scene.objects) {
		glColor3f(1, 1, 1);
		switch (obj.kind) {
			case ObjectKind::Sphere:
				DrawSphere(obj.sphere.origin, obj.sphere.radius);
				break;
		}
	}
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			Scene scene = {};
			new(&scene)Scene();
			Init(scene);
			while (fplWindowUpdate()) {
				fplPollEvents();
				Render(scene);
				fplVideoFlip();
			}
			fglUnloadOpenGL();
		}
		fplPlatformRelease();
	}
	return 0;
}