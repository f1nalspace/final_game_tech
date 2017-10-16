#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

#define FTT_IMPLEMENTATION
#include "final_tiletrace.hpp"

int main(int argc, char **args) {
	using namespace fpl;
	using namespace fpl::memory;
	using namespace fpl::window;

	int result = 0;
	if (InitPlatform(InitFlags::VideoOpenGL)) {
		SetWindowArea(640, 480);
		SetWindowPosition(0, 0);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.39f, 0.58f, 0.93f, 1.0f);

		while (WindowUpdate()) {
			Event ev;
			while (PollWindowEvent(ev)) {
				switch (ev.type) {
					default:
						break;
				}
			}

			WindowSize windowArea = GetWindowArea();

			glViewport(0, 0, windowArea.width, windowArea.height);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 1.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			glColor3f(1.0f, 1.0f, 1.0f);
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, 0.0f);
			glBegin(GL_QUADS);
			glVertex2f(0.5f, 0.5f);
			glVertex2f(-0.5f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();
			glPopMatrix();

			WindowFlip();
		}

		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}