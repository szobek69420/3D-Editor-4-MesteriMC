#include "framework.h"

#include <GL/freeglut.h>	// must be downloaded unless you have an Apple

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glut.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ImGui/imgui_loader.h"

#include "System/system.h"

#include "Editable/editable.h"

#include "ui/header/header.h"

Camera cam;
vec3 camOrigin;

int lastMouseX = 0, lastMouseY = 0;
int middleButtonDown = 0;

void rotateCamera(int dX, int dY);

void ImguiFrame()
{
	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();

	do {
		Header::render();
		Editable::renderHierarchy();
	} while (0);


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

// Initialization, create an OpenGL context
void onInitialization() {
	cam = Camera();
	cam.setPosition(vec3(0, 0, 5.0f));
	camOrigin = vec3(0, 0, 0);
	cam.refreshViewMatrix();


	ImGuiLoader::initialize();
	System::setWindowSize(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowWidth, windowHeight);

	Editable::initialize();
	Editable::add(Editable::Preset::CUBE);

	glViewport(0, 0, windowWidth, windowHeight);
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // clear frame buffer

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);
	Editable::render3D(cam, vec2(0, 0), vec2(windowWidth - 200, windowHeight - Header::HeightInPixels));

	ImguiFrame();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	// ImGui keyboard input processing
	ImGui_ImplGLUT_KeyboardFunc(key, pX, pY);

	glutPostRedisplay();
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
	ImGui_ImplGLUT_KeyboardUpFunc(key, pX, pY);
	glutPostRedisplay();
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// ImGui mouse motion input processing
	System::setMousePosition(pX, pY);
	ImGui_ImplGLUT_MotionFunc(pX, pY);

	rotateCamera(pX-lastMouseX, pY-lastMouseY);

	lastMouseX = pX;
	lastMouseY = pY;

	glutPostRedisplay();
}

void onMouseMotionWithoutClick(int pX, int pY)
{
	//this isnt called when onMouseMotion is
	System::setMousePosition(pX, pY);
	lastMouseX = pX;
	lastMouseY = pY;
	glutPostRedisplay();
}


// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// ImGui mouse input processing
	ImGui_ImplGLUT_MouseFunc(button, state, pX, pY);
	glutPostRedisplay();

	switch (button)
	{
	case GLUT_MIDDLE_BUTTON://middle button
		switch (state)
		{
		case GLUT_DOWN:
			middleButtonDown = 1;
			break;

		case GLUT_UP:
			middleButtonDown = 0;
			break;
		}
		break;
	}
}

void onScroll(int button, int dir, int x, int y)
{
	ImGui_ImplGLUT_MouseWheelFunc(button, dir, x, y);
	glutPostRedisplay();
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	glutPostRedisplay();
}

void onReshape(int width, int height)
{
	System::setWindowSize(width, height);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(width, height);
}

void onDeinitialization()
{
	Editable::deinitialize();
	ImGuiLoader::destroy();
}

void rotateCamera(int dX, int dY)
{
	vec3 vecA = cam.getPosition() - camOrigin;
	float l = length(vecA);
	vec3 vecB = vecA - 0.1f * l *dX* cam.getRight() -0.1f*l*dY*cam.getUp();
	vecA = normalize(vecA);
	vecB = normalize(vecB);

	float deltaYaw = dX > 0 ? dot(vec3(vecA.x, 0, vecA.z), vec3(vecB.x, 0, vecB.z)): -dot(vec3(vecA.x, 0, vecA.z), vec3(vecB.x, 0, vecB.z));
	float deltaPitch = dY > 0 ? dot(vec3(vecA.x, vecB.y, vecA.z), vecA) : -dot(vec3(vecA.x, vecB.y, vecA.z), vecA);

	float yaw, pitch;
	cam.getRotation(&pitch, &yaw);
	yaw += deltaYaw;
	pitch += deltaPitch;
	if (pitch > 89.0f)
		pitch = 89.0f;
	else if (pitch < -89.0f)
		pitch = -89.0f;
	cam.setRotation(pitch, yaw);
	cam.setPosition(camOrigin - l * cam.getDirection());
	cam.refreshViewMatrix();
}