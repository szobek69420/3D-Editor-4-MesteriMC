#include "framework.h"

#include <GL/freeglut.h>	// must be downloaded unless you have an Apple
#include <math.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glut.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ImGui/imgui_loader.h"

#include "System/system.h"

#include "Layout/layout.h"

#include "Editable/editable.h"

#include "ui/header/header.h"
#include "ui/uv_editor/uv_editor.h"

Camera cam, cam2D;
vec3 camOrigin;

Editable* selectedEditable = NULL;
int selectedVertexID = -1;

int lastMouseX = 0, lastMouseY = 0;
int leftButtonDonw = 0;
int middleButtonDown = 0;
int rightButtonDown = 0;
layout_t currentLayout = Layout::NONE;

void rotateCamera(int dX, int dY);
void moveOrigin(int dX, int dY);
void scrollOrigin(int deltaScroll);
void selectPoint3D(int pX, int pY);

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

		vec2 bottomLeft, topRight;
		if(Layout::getLayoutBounds(Layout::UV,&bottomLeft, &topRight))
			UVEditor::render(bottomLeft, topRight);
	} while (0);


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

// Initialization, create an OpenGL context
void onInitialization() {
	cam = Camera();
	cam.setPosition(vec3(3, 2.449466f, 3));
	cam.setRotation(-30, 45);
	cam.refreshViewMatrix();
	camOrigin = vec3(0, 0, 0);

	cam2D = Camera();
	cam2D.setPosition(vec3(0, 0, 5));
	cam2D.refreshViewMatrix();


	ImGuiLoader::initialize();
	System::setWindowSize(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowWidth, windowHeight);

	Layout::setLayout(Layout::Preset::Object);

	Editable::initialize();
	selectedEditable=Editable::add(Editable::Preset::CUBE);

	glViewport(0, 0, windowWidth, windowHeight);
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // clear frame buffer

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	vec2 bottomLeft, topRight;
	if (Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight))
		Editable::render3D(cam, bottomLeft, topRight);
	if (Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight))
		Editable::render2D(cam2D, bottomLeft, topRight);

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

	if (middleButtonDown && currentLayout == Layout::OBJECT)
	{
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			moveOrigin(pX - lastMouseX, pY - lastMouseY);
		else
			rotateCamera(pX - lastMouseX, pY - lastMouseY);
	}

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
	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui_ImplGLUT_MouseFunc(button, state, pX, pY);

	switch (state)
	{
	case GLUT_DOWN:
		currentLayout = Layout::getLayoutByMousePos(pX, windowHeight-pY);
		break;

	case GLUT_UP:
		currentLayout = Layout::NONE;
		break;
	}

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

	if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN)
		selectPoint3D(pX, pY);

	glutPostRedisplay();
}

void onScroll(int button, int dir, int x, int y)
{
	int pX, pY;
	System::getMousePosition(&pX, &pY);
	if (Layout::getLayoutByMousePos(pX, pY) == Layout::OBJECT)
		scrollOrigin(-dir);

	ImGui_ImplGLUT_MouseWheelFunc(button, dir, x, y);
	glutPostRedisplay();
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {

}

void onReshape(int width, int height)
{
	System::setWindowSize(width, height);
	Layout::refresh();
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
	static const float SENSITIVITY_ROT = 0.3f;

	float len = length(cam.getPosition() - camOrigin);
	float pitch, yaw;
	cam.getRotation(&pitch, &yaw);
	yaw -= SENSITIVITY_ROT*dX;
	pitch -= SENSITIVITY_ROT * dY;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	cam.setRotation(pitch, yaw);
	cam.setPosition(camOrigin - len * cam.getDirection());
	cam.refreshViewMatrix();
}

void moveOrigin(int dX, int dY)
{
	static const float SENSITIVITY_MOVE_ORIGIN = 0.01f;

	float len = length(cam.getPosition() - camOrigin);

	camOrigin = camOrigin - dX*SENSITIVITY_MOVE_ORIGIN * cam.getRight() + dY * SENSITIVITY_MOVE_ORIGIN* cam.getUp();
	cam.setPosition(camOrigin - len * cam.getDirection());
	cam.refreshViewMatrix();
}

void scrollOrigin(int deltaScroll)
{
	static const float SENSITIVITY_SCROLL_ORIGIN = 1.05f;

	vec3 vec=cam.getPosition() - camOrigin;
	vec = vec* powf(SENSITIVITY_SCROLL_ORIGIN, deltaScroll);

	cam.setPosition(camOrigin +vec);
	cam.refreshViewMatrix();
}

void selectPoint3D(int pX, int pY)
{
	if (selectedEditable == NULL)
		return;

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	vec2 bottomLeft, topRight;
	if (Layout::getLayoutByMousePos(pX, pY) != Layout::OBJECT || Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight) == 0)
		return;
	
	vec2 ndc = vec2(
		2.0f * (pX - bottomLeft.x) / (topRight.x - bottomLeft.x) - 1,
		2.0f * (pY - bottomLeft.y-(windowHeight-topRight.y)) / (topRight.y - bottomLeft.y) - 1);

	float szam = sinf(0.01745329252f * 0.5f*Camera::getFov());
	float szam2 = (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y);

	ndc.x *= szam*szam2;
	ndc.y *= szam*(-1);

	vec3 raycastDir = normalize(vec3(ndc.x, ndc.y, -cosf(0.01745329252f * 0.5f*Camera::getFov())));
	mat4 mv = selectedEditable->getGlobalMatrix()*cam.getViewMatrix();
	const std::vector<VertexData>& vertices = selectedEditable->getVertices();

	int closest = -1;
	float minDistance = 1000000;
	for (int i = 0; i < vertices.size(); i++)
	{
		vec4 vertexPosTemp = vec4(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, 1) *  mv;
		vec3 vertexPos = vec3(vertexPosTemp.x, vertexPosTemp.y, vertexPosTemp.z);

		float distance = sqrtf(
			powf(length(vertexPos),2)-
			powf(dot(vertexPos,raycastDir),2)
		);

		if (distance<0.1f&&distance < minDistance)
		{
			closest = i;
			minDistance = distance;
		}
	}
	selectedVertexID = closest;
	
}