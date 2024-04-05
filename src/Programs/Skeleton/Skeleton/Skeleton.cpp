#include "framework.h"

#include <GL/freeglut.h>	// must be downloaded unless you have an Apple

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glut.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ImGui/imgui_loader.h"

#include "System/system.h"

#include "Editable/editable.h"

#include "ui/header/header.h"

//shader
const char* vs = "\n"
"#version 330 core \n"
"precision highp float;\n"
"layout(location = 0) in vec2 aPos; \n"
"layout(location = 1) in vec2 aUV; \n"
"out vec2 uv; \n"
"uniform mat4 model; \n"
"uniform mat4 view; \n"
"uniform mat4 projection; \n"
"void main() { \n"
"	uv=aUV; \n"
"	gl_Position = vec4(aPos, 0, 1)*model*view*projection; \n"
"}\0";
const char* fs = "\n"
"#version 330 core	\n"
"precision highp float;\n"
"in vec2 uv; \n"
"out vec4 fragColor;	\n"
"uniform sampler2D roblox; \n"
"void main() { \n"
"	fragColor = texture(roblox, uv); \n"
"}\0";


GPUProgram program;

int inAnimation = 0;
float animationTime = 0;
float lastAnimationFrame = 0;

float sliderValue = 0.5f;
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
	ImGuiLoader::initialize();
	System::setWindowSize(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowWidth, windowHeight);

	Editable::initialize();

	glViewport(0, 0, windowWidth, windowHeight);


	program.create(vs, fs, "fragColor");

}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer


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
	glutPostRedisplay();
}

void onMouseMotionWithoutClick(int pX, int pY)
{
	//this isnt called when onMouseMotion is
	System::setMousePosition(pX, pY);
	glutPostRedisplay();
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// ImGui mouse input processing
	ImGui_ImplGLUT_MouseFunc(button, state, pX, pY);
	glutPostRedisplay();
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