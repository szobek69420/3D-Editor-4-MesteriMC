#include "framework.h"

#include <GL/freeglut.h>	// must be downloaded unless you have an Apple
#include <math.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glut.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ImGui/imgui_loader.h"

#include "System/system.h"

#include "Layout/layout.h"
#include "Grid/grid.h"

#include "Editable/editable.h"

#include "ui/header/header.h"
#include "ui/uv_editor/uv_editor.h"

enum Operation {
	NONE,
	MOVE_OBJECT, MOVE_VERTEX, MOVE_VERTEX_UV,
	SCALE_OBJECT,
};

class OperationRollbackItemObject {
public:
	Editable* edible;
	vec3 position;
	vec3 rotation;
	vec3 scale;

	OperationRollbackItemObject(Editable* edible, const vec3& position, const vec3& rotation, const vec3& scale)
	{
		this->edible = edible;
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}
};

class OperationRollbackItemVertex {
public:
	struct VertexData data;
	unsigned int vertexID;

	OperationRollbackItemVertex(struct VertexData data, unsigned int vertexID)
	{
		this->data = data;
		this->vertexID = vertexID;
	}
};

Camera cam, cam2D;
vec3 camOrigin;
float cam2Dzoom = 2.0f;

std::vector<Editable*> editablesInScene;
Editable* selectedEditable = NULL;
std::vector<unsigned int> selectedVertexIDs;
int showVertices = 0;

int lastMouseX = 0, lastMouseY = 0;
int leftButtonDonw = 0;
int middleButtonDown = 0;
int rightButtonDown = 0;
layout_t currentLayout = Layout::NONE;
int currentOperation = Operation::NONE;
std::vector<OperationRollbackItemObject> operationRollbackObject;
std::vector<OperationRollbackItemVertex> operationRollbackVertex;
float operationHelper11; vec2 operationHelper21; vec3 operationHelper31;
float operationHelper12; vec2 operationHelper22; vec3 operationHelper32;

void rotateCamera(int dX, int dY);
void moveOrigin(int dX, int dY);
void scrollOrigin(int deltaScroll);
void selectPoint3D(int pX, int pY, int append);
void selectObject3D(int pX, int pY);

void move2D(int dX, int dY);
void scroll2D(int deltaScroll);
void selectPoint2D(int pX, int pY, int append);

void startOperation(Operation op);
void endOperation(int discard = 0);
void processOperation(int dX, int dY);

void ImguiFrame();

// Initialization, create an OpenGL context
void onInitialization() {
	cam = Camera();
	cam.setPosition(vec3(3, 2.449466f, 3));
	cam.setRotation(-30, 45);
	cam.refreshViewMatrix();
	camOrigin = vec3(0, 0, 0);

	cam2D = Camera();
	cam2D.setPosition(vec3(0.5f, 0.5f, 1));
	cam2D.refreshViewMatrix();


	ImGuiLoader::initialize();
	System::setWindowSize(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowWidth, windowHeight);

	Grid::initialize();

	Layout::setLayout(Layout::Preset::Object);

	Editable::initialize();
	editablesInScene.push_back(Editable::add(Editable::Preset::CUBE));

	glViewport(0, 0, windowWidth, windowHeight);
}

void onDeinitialization()
{
	Grid::deinitialize();
	Editable::deinitialize();
	ImGuiLoader::destroy();
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // clear frame buffer

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	vec2 bottomLeft, topRight;
	if (Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight))
	{
		bottomLeft = System::convertScreenToGl(bottomLeft);
		topRight = System::convertScreenToGl(topRight);

		Grid::setColour(0.3f, 0.3f, 0.3f);
		Grid::setStepSize(1);
		Grid::render(200, bottomLeft, topRight, cam, -100);
		Editable::render3D(cam, bottomLeft, topRight, showVertices);
	}
	if (Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight))
		Editable::render2D(cam2D, System::convertScreenToGl(bottomLeft), System::convertScreenToGl(topRight), cam2Dzoom);



	ImguiFrame();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	
	endOperation(69);

	switch(key)
	{
		case 'g':
			switch (Layout::getLayoutByMousePos(pX, pY))
			{
			case Layout::OBJECT:
				if (showVertices == 0)//object mode
				{
					if (selectedEditable != NULL)
						startOperation(Operation::MOVE_OBJECT);
				}
				else//edit mode
				{
					if (selectedVertexIDs.size() != 0)
						startOperation(Operation::MOVE_VERTEX);
				}
				break;

			case Layout::UV:
				if (selectedVertexIDs.size() != 0)
					startOperation(Operation::MOVE_VERTEX_UV);
				break;
			}
			break;

		case 's':
			switch (Layout::getLayoutByMousePos(pX, pY))
			{
			case Layout::OBJECT:
				if (showVertices == 0)//object mode
				{
					if (selectedEditable != NULL)
						startOperation(Operation::SCALE_OBJECT);
				}
				else//edit mode
				{

				}
				break;

			case Layout::UV:

				break;
			}
			break;

		case '\t':
			selectedVertexIDs.clear();
			if(selectedEditable!=NULL)
				showVertices = 1 - showVertices;
			break;
	}
	

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

	if (middleButtonDown && currentLayout == Layout::UV)
		move2D(pX - lastMouseX, pY - lastMouseY);

	lastMouseX = pX;
	lastMouseY = pY;

	glutPostRedisplay();
}

void onMouseMotionWithoutClick(int pX, int pY)//this isnt called when onMouseMotion is
{
	processOperation(pX - lastMouseX, pY - lastMouseY);


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
		currentLayout = Layout::getLayoutByMousePos(pX, pY);
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

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		endOperation();
		
		if (showVertices == 0)//object mode
			selectObject3D(pX, pY);
		else//edit mode
			selectPoint3D(pX, pY, glutGetModifiers() == GLUT_ACTIVE_SHIFT ? 69 : 0);

		selectPoint2D(pX, pY, glutGetModifiers() == GLUT_ACTIVE_SHIFT ? 69 : 0);
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		endOperation(69);
	}

	glutPostRedisplay();
}

void onScroll(int button, int dir, int x, int y)
{
	int pX, pY;
	System::getMousePosition(&pX, &pY);
	switch (Layout::getLayoutByMousePos(pX, pY))
	{
		case Layout::OBJECT:
			scrollOrigin(-dir);
			break;

		case Layout::UV:
			scroll2D(-dir);
			break;
	}


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
		if (Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight))
			UVEditor::render(bottomLeft, topRight);
	} while (0);


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

//3d functinos

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

void selectPoint3D(int pX, int pY, int append)
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
		2.0f * (pY - topRight.y) / (bottomLeft.y- topRight.y) - 1);

	float szam = sinf(0.01745329252f * 0.5f*Camera::getFov());
	float szam2 = (topRight.x - bottomLeft.x) / (bottomLeft.y- topRight.y);

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

		if (vertexPos.z<0&&distance<0.1f&&distance < minDistance)
		{
			closest = i;
			minDistance = distance;
		}
	}
	
	if (closest == -1)
		selectedVertexIDs.clear();
	else
	{
		if (append != 0)
		{
			char contains = 0;
			for (int i = 0; i < selectedVertexIDs.size(); i++)
			{
				if (selectedVertexIDs[i] == closest)
				{
					contains = 69;
					break;
				}
			}
			if (contains == 0)
				selectedVertexIDs.push_back(closest);
		}
		else
		{
			selectedVertexIDs.clear();
			selectedVertexIDs.push_back(closest);
		}
	}
}

void selectObject3D(int pX, int pY)
{
	vec2 bottomLeft, topRight;
	if (Layout::getLayoutByMousePos(pX, pY) != Layout::OBJECT || Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight) == 0)
		return;

	vec3 ndc = vec3(
		2.0f * (pX - bottomLeft.x) / (topRight.x - bottomLeft.x) - 1,
		-2.0f * (pY - topRight.y) / (bottomLeft.y - topRight.y) + 1,
		0);

	mat4 vp = cam.getViewMatrix()*PerspectiveMatrix(60, (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y), 0.01f, 100.0f);


	int closest = -1;
	float minZ = 100000;
	for (int j = 0; j < editablesInScene.size(); j++)
	{
		const std::vector<VertexData>& vertices = editablesInScene[j]->getVertices();
		const std::vector<unsigned int>& indices = editablesInScene[j]->getIndices();

		mat4 mvp = editablesInScene[j]->getGlobalMatrix()*vp;

		for (int i = 0; i < indices.size(); i += 3)
		{
			vec4 a4 = vertices[indices[i]].position*mvp, b4 = vertices[indices[i + 1]].position*mvp, c4 = vertices[indices[i + 2]].position*mvp;
			vec3 a = a4, b = b4, c = c4;

			float avgZ = 0.3333f * (a.z + b.z + c.z);
			if (avgZ < 0)
				continue;

			a = a / a4.w; b = b / b4.w; c = c / c4.w;//perspective division
		

			a.z = 0; b.z = 0; c.z = 0;
			int positives = 0;
			if (cross(ndc - a, b - a).z > 0)
				positives++;
			if (cross(ndc - b, c - b).z > 0)
				positives++;
			if (cross(ndc - c, a - c).z > 0)
				positives++;

			if (positives != 0 && positives != 3)
				continue;

			if (avgZ < minZ)
			{
				closest = j;
				minZ = avgZ;
			}
		}
	}
	if (closest == -1)
		selectedEditable = NULL;
	else
		selectedEditable = editablesInScene[closest];
}


//2d functions
void scroll2D(int deltaScroll)
{
	static const float SENSITIVITY_SCROLL_2D = 1.05f;

	cam2Dzoom *=  powf(SENSITIVITY_SCROLL_2D, deltaScroll);

	if (cam2D.getPosition().z < 0.01f)
		cam2D.getPosition().z = 0.11f;
	else if (cam2D.getPosition().z > 99.0f)
		cam2D.getPosition().z = 99.0f;

	cam2D.refreshViewMatrix();
}

void move2D(int dX, int dY)
{
	float SENSITIVITY_MOVE_2D = cam2Dzoom*0.002f;
	cam2D.setPosition(cam2D.getPosition() - SENSITIVITY_MOVE_2D * vec3(dX, -dY,0));
	cam2D.refreshViewMatrix();
}

void selectPoint2D(int pX, int pY, int append)
{
	if (selectedEditable == NULL)
		return;

	vec2 bottomLeft, topRight;
	if (Layout::getLayoutByMousePos(pX, pY) != Layout::UV || Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight) == 0)
		return;

	vec2 ndc = vec2(
		2.0f * (pX - bottomLeft.x) / (topRight.x - bottomLeft.x) - 1,
		-2.0f * (pY - topRight.y) / (bottomLeft.y- topRight.y) + 1);


	float aspectXY = (topRight.x - bottomLeft.x) / (bottomLeft.y- topRight.y);
	mat4 vp = cam2D.getViewMatrix()* OrthoMatrix(-0.5f * aspectXY * cam2Dzoom, 0.5f * aspectXY * cam2Dzoom, -0.5f * cam2Dzoom, 0.5f * cam2Dzoom, 0, 10.0f);
	const std::vector<VertexData>& vertices = selectedEditable->getVertices();

	int closest = -1;
	float minDistance = 1000000;
	for (int i = 0; i < vertices.size(); i++)
	{
		vec4 vertexPosTemp = vec4(vertices[i].uv.x, vertices[i].uv.y, 0, 1) * vp;
		vec2 vertexPos = vec2(vertexPosTemp.x/vertexPosTemp.w, vertexPosTemp.y/vertexPosTemp.w);

		float distance = length(ndc-vertexPos);

		if (distance < minDistance)
		{
			if(distance<0.1f)
				closest = i;
			minDistance = distance;
		}
	}

	if (closest == -1)
		selectedVertexIDs.clear();
	else
	{
		if (append != 0)
		{
			char contains = 0;
			for (int i = 0; i < selectedVertexIDs.size(); i++)
			{
				if (selectedVertexIDs[i] == closest)
				{
					contains = 69;
					break;
				}
			}
			if (contains == 0)
				selectedVertexIDs.push_back(closest);
		}
		else
		{
			selectedVertexIDs.clear();
			selectedVertexIDs.push_back(closest);
		}
	}
}

//operation
void startOperation(Operation op)
{
	if (selectedEditable == NULL || (selectedVertexIDs.size() == 0 && showVertices!=0))
		return;

	operationRollbackObject.push_back(OperationRollbackItemObject(selectedEditable, selectedEditable->getPosition(), selectedEditable->getRotation(), selectedEditable->getScale()));

	const std::vector<VertexData>& vertices = selectedEditable->getVertices();
	for(int i=0;i<selectedVertexIDs.size();i++)
		operationRollbackVertex.push_back(OperationRollbackItemVertex(vertices[selectedVertexIDs[i]], selectedVertexIDs[i]));

	currentOperation = op;

	//helpers
	int pX, pY;
	System::getMousePosition(&pX, &pY);

	vec2 bottomLeft, topRight;
	switch (currentOperation)
	{
	case SCALE_OBJECT:
		Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);
		bottomLeft = System::convertScreenToGl(bottomLeft);
		topRight = System::convertScreenToGl(topRight);
		vec4 temp = vec4(0, 0, 0, 1) * selectedEditable->getGlobalMatrix()*cam.getViewMatrix()* PerspectiveMatrix(60, (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y), 0.01f, 100.0f);
		operationHelper21 = bottomLeft+vec2((topRight.x-bottomLeft.x)*temp.x / temp.w, (topRight.y - bottomLeft.y) * temp.y / temp.w);//position of object on screen
		operationHelper22 = vec2(pX, pY);
		break;
	}
}

void endOperation(int discard)
{
	if (discard != 0)
	{
		for (int i = 0; i < operationRollbackObject.size(); i++)
		{
			operationRollbackObject[i].edible->setPosition(operationRollbackObject[i].position);
			operationRollbackObject[i].edible->setScale(operationRollbackObject[i].scale);
			operationRollbackObject[i].edible->setRotation(operationRollbackObject[i].rotation);
			operationRollbackObject[i].edible->recalculateGlobalMatrix();
		}

		for (int i = 0; i < operationRollbackVertex.size(); i++)
		{
			selectedEditable->setVertexData(operationRollbackVertex[i].vertexID, operationRollbackVertex[i].data);
		}
	}
	printf("amogus2\n");
	operationRollbackObject.clear();
	operationRollbackVertex.clear();
	currentOperation = Operation::NONE;
}

void processOperation(int dX, int dY)
{
	VertexData vd;
	vec2 delta2;
	vec3 delta3;
	switch (currentOperation)
	{
	case Operation::MOVE_VERTEX_UV:
		delta2= 0.002f * cam2Dzoom * vec2(dX, -dY);
		for (int i = 0; i < selectedVertexIDs.size(); i++)
		{
			vd = selectedEditable->getVertices()[selectedVertexIDs[i]];
			vd.uv = vd.uv + delta2;
			selectedEditable->setVertexData(selectedVertexIDs[i], vd);
		}
		break;

	case Operation::MOVE_VERTEX:
		delta3 = 0.00415f * dX * cam.getRight() - 0.00415f * dY * cam.getUp();
		for (int i = 0; i < selectedVertexIDs.size(); i++)
		{
			vd = selectedEditable->getVertices()[selectedVertexIDs[i]];
			vd.position = vd.position + delta3;
			selectedEditable->setVertexData(selectedVertexIDs[i], vd);
		}
		break;

	case Operation::MOVE_OBJECT:
		delta3 = 0.00415f * dX * cam.getRight() - 0.00415f * dY * cam.getUp();
		selectedEditable->setPosition(selectedEditable->getPosition() + delta3);
		selectedEditable->recalculateGlobalMatrix();
		break;

	case Operation::SCALE_OBJECT:
		delta3 = 0.00415f * dX * cam.getRight() - 0.00415f * dY * cam.getUp();
		selectedEditable->setPosition(selectedEditable->getPosition() + delta3);
		selectedEditable->recalculateGlobalMatrix();
		break;
	}
}