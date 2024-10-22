#include <crtdbg.h>
#include <ShObjIdl.h>
#include <objbase.h>

#include "framework.h"
#include "Quaternion/quaternion.h"

#include <GL/freeglut.h>	// must be downloaded unless you have an Apple
#include <math.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glut.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ImGui/imgui_loader.h"

#include "System/system.h"

#include "Layout/layout.h"
#include "Grid/grid.h"
#include "OrientationIcon/orientation_icon.h"
#include "Axis/axis.h"

#include "Editable/editable.h"
#include "Rollback/rollback.h"

#include "ui/header/header.h"
#include "ui/uv_editor/uv_editor.h"
#include "ui/object_local/object_local_list.h"

#include "Export/export.h"

#define DEG2RAD 0.01745329252f

enum Operation {
	NONE,
	MOVE_OBJECT, MOVE_VERTEX, MOVE_VERTEX_UV,
	SCALE_OBJECT, SCALE_VERTEX, SCALE_VERTEX_UV,
	ROTATE_OBJECT, ROTATE_VERTEX, ROTATE_VERTEX_UV
};

enum OperationDirection {
	DIR_ALL=-1, DIR_X=0, DIR_Y=1, DIR_Z=2
};
typedef OperationDirection OD;

class OperationRollbackItemObject {
public:
	Editable* edible;
	vec3 position;
	quat rotation;
	vec3 scale;

	OperationRollbackItemObject(Editable* edible, const vec3& position, const quat& rotation, const vec3& scale)
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
int showVertexNormals = 0;

int lastMouseX = 0, lastMouseY = 0;
int leftButtonDonw = 0;
int middleButtonDown = 0;
int rightButtonDown = 0;
layout_t currentLayout = Layout::NONE;
int currentOperation = Operation::NONE;
int currentOperationDirection = OperationDirection::DIR_ALL; vec3 currentOperationAxisCenter;
std::vector<OperationRollbackItemObject> operationRollbackObject;
std::vector<OperationRollbackItemVertex> operationRollbackVertex;
float operationHelper11; vec2 operationHelper21; vec3 operationHelper31; quat operationHelperQ1;
float operationHelper12; vec2 operationHelper22; vec3 operationHelper32;
float operationCenterFromCamera;//only used for the mouse sensitivity calculation, do not use elsewhere

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
vec3 calculateOperationCenter();
vec2 calculateMouseSensitivity3D(const vec2& bottomLeft, const vec2& topRight);
vec2 calculateMouseSensitivity2D(const vec2& bottomLeft, const vec2& topRight);

void ImguiFrame();
void onDeinitialization();

void closeAllLocalLists();

// Initialization, create an OpenGL context
void onInitialization() {
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);//so that onDeinitialization is called
	atexit(onDeinitialization);

	CoInitialize(NULL);

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
	OrientationIcon::initialize();
	Axis::initialize();

	Layout::setLayout(Layout::Preset::Object);

	Editable::initialize();
	editablesInScene.push_back(Editable::add(Editable::Preset::CUBE));

	glViewport(0, 0, windowWidth, windowHeight);
}

void onDeinitialization()
{
	Grid::deinitialize();
	OrientationIcon::deinitialize();
	Axis::deinitialize();
	Editable::deinitialize();
	ImGuiLoader::destroy();

	CoUninitialize();
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear frame buffer

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	vec2 bottomLeft, topRight;
	if (Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight))
	{
		bottomLeft = System::convertScreenToGl(bottomLeft);
		topRight = System::convertScreenToGl(topRight);

		float stepSize = powf(10, floorf(log10f(0.5f*length(cam.getPosition() - camOrigin))));
		if (stepSize < 1)
			stepSize = 1;
		Grid::setStepSize(stepSize);
		Grid::setColour(0.3f, 0.3f, 0.3f);
		Grid::render(200, bottomLeft, topRight, cam, -100*stepSize);
		Editable::render3D(cam, bottomLeft, topRight, showVertices, showVertexNormals);
		switch (currentOperationDirection)
		{
			case OD::DIR_X: Axis::render(Axis::Direction::DIR_X, cam, currentOperationAxisCenter, bottomLeft, topRight); break;
			case OD::DIR_Y: Axis::render(Axis::Direction::DIR_Y, cam, currentOperationAxisCenter, bottomLeft, topRight); break;
			case OD::DIR_Z: Axis::render(Axis::Direction::DIR_Z, cam, currentOperationAxisCenter, bottomLeft, topRight); break;
		}

		if (topRight.x - bottomLeft.x > 100 && topRight.y - bottomLeft.y > 100)
			OrientationIcon::render(cam, topRight - vec2(80, 80), topRight - vec2(20, 20));
	}
	if (Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight))
		Editable::render2D(cam2D, System::convertScreenToGl(bottomLeft), System::convertScreenToGl(topRight), cam2Dzoom);



	ImguiFrame();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	
	ImGui_ImplGLUT_KeyboardFunc(key, pX, pY);

	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		glutPostRedisplay();
		return;
	}

	if (currentOperation != Operation::NONE)
	{
		switch (key)//check for inputs that are connected to the current operation
		{
			case 'x':
				if (Layout::getLayoutByMousePos(pX, pY)!=Layout::OBJECT)
					break;
				currentOperationDirection = currentOperationDirection == OperationDirection::DIR_X ? OperationDirection::DIR_ALL : OperationDirection::DIR_X;
				currentOperationAxisCenter = calculateOperationCenter();
				glutPostRedisplay();
				return;

			case 'y':
				if (Layout::getLayoutByMousePos(pX, pY) != Layout::OBJECT)
					break;
				currentOperationDirection = currentOperationDirection == OperationDirection::DIR_Y ? OperationDirection::DIR_ALL : OperationDirection::DIR_Y;
				currentOperationAxisCenter = calculateOperationCenter();
				glutPostRedisplay();
				return;

			case 'z':
				if (Layout::getLayoutByMousePos(pX, pY) != Layout::OBJECT)
					break;
				currentOperationDirection = currentOperationDirection == OperationDirection::DIR_Z ? OperationDirection::DIR_ALL : OperationDirection::DIR_Z;
				currentOperationAxisCenter = calculateOperationCenter();
				glutPostRedisplay();
				return;
		}
	}

	endOperation(69);

	switch(key)
	{
		case 'g': //move
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

		case 's': //scale
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
					if (selectedVertexIDs.size()  > 1)
						startOperation(Operation::SCALE_VERTEX);
				}
				break;

			case Layout::UV:
				if (selectedVertexIDs.size() > 1)
					startOperation(Operation::SCALE_VERTEX_UV);
				break;
			}
			break;

		case 'r': //rotate
			switch (Layout::getLayoutByMousePos(pX, pY))
			{
			case Layout::OBJECT:
				if (showVertices == 0)//object mode
				{
					if (selectedEditable != NULL)
						startOperation(Operation::ROTATE_OBJECT);
				}
				else//edit mode
				{
					if (selectedVertexIDs.size() > 1)
						startOperation(Operation::ROTATE_VERTEX);
				}
				break;

			case Layout::UV:
				if (selectedVertexIDs.size() > 1)
					startOperation(Operation::ROTATE_VERTEX_UV);
				break;
			}
			break;

		case 'a': //select all
			switch (Layout::getLayoutByMousePos(pX, pY))
			{
			case Layout::OBJECT:
			case Layout::UV:
				if (showVertices == 1 && selectedEditable != NULL)
				{
					selectedVertexIDs.clear();
					const std::vector<VertexData>& vertices = selectedEditable->getVertices();
					for (unsigned int i = 0; i < vertices.size(); i++)
						selectedVertexIDs.push_back(i);
				}
				break;
			}
			
			break;

		case 'd': //duplicate
			if (Layout::getLayoutByMousePos(pX, pY) == Layout::OBJECT&&selectedEditable!=NULL)
			{
				if (showVertices == 0) //object mode
				{
					selectedEditable = Editable::clone(selectedEditable);
					editablesInScene.push_back(selectedEditable);

					startOperation(Operation::MOVE_OBJECT);
				}
				else //edit mode
				{
					if (selectedVertexIDs.size() == 0)
						break;

					const std::vector<VertexData>& vertices = selectedEditable->getVertices();
					unsigned int orgVertexCount = vertices.size();
					for (int i = 0; i < selectedVertexIDs.size(); i++)
					{
						//add new data
						selectedEditable->addVertex(vertices[selectedVertexIDs[i]].position, vertices[selectedVertexIDs[i]].uv);
						//refresh vertex id
						selectedVertexIDs[i] = orgVertexCount + i;
					}

					startOperation(Operation::MOVE_VERTEX);
				}
			}
			break;

		case 'x': //delete
			if (Layout::getLayoutByMousePos(pX, pY) == Layout::OBJECT && selectedEditable != NULL)
			{
				if (showVertices == 0) //object mode
				{
					RollbackItem::addToBuffer(RollbackDeleteObject("delete", selectedEditable));
					//remove selectedEditable from editablesInScene
					for(int i=0;i<editablesInScene.size();i++)
						if (editablesInScene[i] == selectedEditable)
						{
							editablesInScene.erase(editablesInScene.begin() + i);
							break;
						}

					Editable::remove(selectedEditable);

					selectedEditable = NULL;
					selectedVertexIDs.clear();
				}
				else //edit mode
				{
					if (selectedVertexIDs.size() == 0)
						break;

					//sort the selected indices in a decending order, so that the values selectedVertexIDs won't become invalid if one of them is deleted
					for (int i = 0; i < selectedVertexIDs.size(); i++)
					{
						for (int j = 0; j < selectedVertexIDs.size() - 1 - i; j++)
						{
							if (selectedVertexIDs[j] < selectedVertexIDs[j + 1])
							{
								unsigned int temp = selectedVertexIDs[j];
								selectedVertexIDs[j] = selectedVertexIDs[j + 1];
								selectedVertexIDs[j + 1] = temp;
							}
						}
					}

					RollbackComposite commands = RollbackComposite("delete");
					RollbackOrientationVertex command1 = RollbackOrientationVertex("delete vertices", selectedEditable->getId(), selectedEditable->getVertices(), selectedEditable->getIndices());
					commands.addItem(&command1);

					for (int i = 0; i < selectedVertexIDs.size(); i++)
						selectedEditable->removeVertex(selectedVertexIDs[i]);

					selectedVertexIDs.clear();

					if (selectedEditable->getVertices().size() == 0)
					{
						RollbackDeleteObject command2 = RollbackDeleteObject("delete object", selectedEditable);
						commands.addItem(&command2);
						Editable::remove(selectedEditable);
						selectedEditable == NULL;
						showVertices = 0;
					}

					RollbackItem::addToBuffer(commands);
				}
			}
			break;

		case 'f': //create face
			if (Layout::getLayoutByMousePos(pX, pY) == Layout::OBJECT 
				&& selectedEditable != NULL
				&& selectedVertexIDs.size()==3
				&& showVertices!=0)
			{
				const std::vector<VertexData>& vertices = selectedEditable->getVertices();

				//check in which direction should the normal face
				mat4 mv = selectedEditable->getGlobalMatrix() * cam.getViewMatrix();

				vec3 a_t = vertices[selectedVertexIDs[0]].position * mv;
				vec3 b_t = vertices[selectedVertexIDs[1]].position * mv;
				vec3 c_t = vertices[selectedVertexIDs[2]].position * mv;

				vec3 kreuzProdukt = cross(a_t - b_t, c_t - b_t);
				if (kreuzProdukt.z < 0)
					selectedEditable->addFace(selectedVertexIDs[0], selectedVertexIDs[1], selectedVertexIDs[2]);
				else
					selectedEditable->addFace(selectedVertexIDs[1], selectedVertexIDs[0], selectedVertexIDs[2]);
			}
			break;

		case '\t': //switch between edit and object modes
			selectedVertexIDs.clear();
			if (selectedEditable != NULL)
				showVertices = 1 - showVertices;
			break;

		case 'n'://show normals
			if (showVertices != 0)
				showVertexNormals = 1 - showVertexNormals;
			break;

		case 'z'://rollback
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL||69)
			{
				RollbackItem::undo();
			}
			break;

		case 't'://test export
			Editable * *edibles = (Editable**)malloc(sizeof(Editable*));
			for(int i=0;i<editablesInScene.size();i++)
				if (editablesInScene[i]->getParent() == 0)
				{
					edibles[0] = editablesInScene[i];
					Exporter::exportEditable("./morbius.morbius", edibles, 1);
					printf("done\n");
					break;
				}
			free(edibles);
			break;
	}

	glutPostRedisplay();
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
	ImGui_ImplGLUT_KeyboardUpFunc(key, pX, pY);
	glutPostRedisplay();
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	System::setMousePosition(pX, pY);//ez legyen az elso

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
	System::setMousePosition(pX, pY);//ez legyen az elso

	processOperation(pX - lastMouseX, pY - lastMouseY);


	lastMouseX = pX;
	lastMouseY = pY;
	glutPostRedisplay();
}


// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	ImGui_ImplGLUT_MouseFunc(button, state, pX, pY);
	if (ImGui::GetIO().WantCaptureMouse)
	{
		glutPostRedisplay();
		return;
	}

	if(state==GLUT_DOWN)
		closeAllLocalLists();

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

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
		if(currentOperation!=Operation::NONE)
			endOperation();
		else {
			if (showVertices == 0)//object mode
				selectObject3D(pX, pY);
			else//edit mode
			{
				selectPoint3D(pX, pY, glutGetModifiers() == GLUT_ACTIVE_SHIFT ? 69 : 0);
				selectPoint2D(pX, pY, glutGetModifiers() == GLUT_ACTIVE_SHIFT ? 69 : 0);
			}
		}
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (currentOperation==Operation::NONE&&Layout::getLayoutByMousePos(pX, pY) == Layout::OBJECT)
			ObjectLocalList::open(vec2(pX, pY));

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
	static vec2 bottomLeft, topRight;
	Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);

	static const vec2 MOUSE_SENSITIVITY_AT_1080P_60FOV = vec2(0.0034f, 0.0034f);
	vec2 ratio = vec2(1920.0f / fabsf(topRight.x - bottomLeft.x), 1080.0f / fabsf(topRight.y - bottomLeft.y));
	ratio.x *= fabsf((topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y)) * 0.5625f;//0.5625 is the 1/aspectXY @ 1080p
	ratio = ratio * (0.5f / sinf(DEG2RAD * 0.5f * cam.getFov()));

	//distance from camera
	float len = length(cam.getPosition() - camOrigin);
	ratio = ratio * (len * 0.001f);

	camOrigin = camOrigin - dX*ratio.x * cam.getRight() + dY * ratio.y* cam.getUp();
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

	float szam = sinf(DEG2RAD * 0.5f*cam.getFov());
	float szam2 = (topRight.x - bottomLeft.x) / (bottomLeft.y- topRight.y);

	ndc.x *= szam*szam2;
	ndc.y *= szam*(-1);

	vec3 raycastDir = normalize(vec3(ndc.x, ndc.y, -cosf(0.01745329252f * 0.5f*cam.getFov())));
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

	mat4 vp = cam.getViewMatrix()*cam.getPerspective((topRight.x - bottomLeft.x) / (bottomLeft.y- topRight.y));


	int closest = -1;
	float minZ = 100000;
	for (int j = 0; j < editablesInScene.size(); j++)
	{
		const std::vector<VertexData>& vertices = editablesInScene[j]->getVertices();
		const std::vector<unsigned int>& indices = editablesInScene[j]->getIndices();

		mat4 mvp = editablesInScene[j]->getGlobalMatrix()*vp;

		for (int i = 0; i < indices.size(); i += 3)
		{
			vec4 a4 = vec4(vertices[indices[i]].position)*mvp, b4 = vec4(vertices[indices[i + 1]].position)*mvp, c4 = vec4(vertices[indices[i + 2]].position)*mvp;
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
	{
		vec3 temp = cam2D.getPosition();
		temp.z = 0.11f;
		cam2D.setPosition(temp);
	}
	else if (cam2D.getPosition().z > 99.0f)
	{
		vec3 temp = cam2D.getPosition();
		temp.z = 99.0f;
		cam2D.setPosition(temp);
	}

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
	currentOperationDirection = OperationDirection::DIR_ALL;

	//rollback
	static char operationName[ROLLBACK_MAX_NAME_LENGTH];
	switch (currentOperation)
	{
		case MOVE_OBJECT:		strcpy(operationName, "move");		break;
		case SCALE_OBJECT:		strcpy(operationName, "scale");		break;
		case ROTATE_OBJECT:		strcpy(operationName, "rotation");	break;
		case MOVE_VERTEX:		strcpy(operationName, "move");		break;
		case SCALE_VERTEX:		strcpy(operationName, "scale");		break;
		case ROTATE_VERTEX:		strcpy(operationName, "rotation");	break;
		case MOVE_VERTEX_UV:	strcpy(operationName, "move");		break;
		case SCALE_VERTEX_UV:	strcpy(operationName, "scale");		break;
		case ROTATE_VERTEX_UV:	strcpy(operationName, "rotation");	break;
	}

	switch (currentOperation)
	{
	case MOVE_OBJECT:
	case SCALE_OBJECT:
	case ROTATE_OBJECT:
		RollbackItem::addToBuffer(RollbackOrientationObject(operationName, selectedEditable->getId(), selectedEditable->getPosition(), selectedEditable->getScale(), selectedEditable->getRotation()));
		break;

	case MOVE_VERTEX:
	case SCALE_VERTEX:
	case ROTATE_VERTEX:
	case MOVE_VERTEX_UV:
	case SCALE_VERTEX_UV:
	case ROTATE_VERTEX_UV:
		RollbackItem::addToBuffer(RollbackOrientationVertex(operationName, selectedEditable->getId(), selectedEditable->getVertices(), selectedEditable->getIndices()));
		break;
	}

	//helpers
	int pX, pY;
	System::getMousePosition(&pX, &pY);

	vec2 bottomLeft, topRight;
	switch (currentOperation)
	{
	case MOVE_OBJECT:
		//operationHelper31: the overall movement

		operationCenterFromCamera = length(selectedEditable->getPosition()-cam.getPosition());
		operationHelper31 = vec3();
		break;

	case MOVE_VERTEX:
		//operationHelper31: the overall movement

		do {
			vec3 temp = vec3();
			const std::vector<VertexData>& vertices = selectedEditable->getVertices();
			for (auto& i = selectedVertexIDs.begin(); i < selectedVertexIDs.end(); i++)
				temp = temp + vertices[*i].position;
			if (selectedVertexIDs.size() > 0)
				temp = temp / selectedVertexIDs.size();

			operationCenterFromCamera = length(temp-cam.getPosition());
		} while (0);

		operationHelper31 = vec3();
		break;

	case MOVE_VERTEX_UV:
		//operationHelper21: the overall movement

		operationHelper21 = vec2();
		break;

	case SCALE_OBJECT:
		//operationHelper21: the screen space position of the center of the object
		//operationHelper22: the mouse position in screen space at the start of the operation
		//operationHelper31: the initial scale of the object

		do {
			Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);
			bottomLeft = System::convertScreenToGl(bottomLeft);
			topRight = System::convertScreenToGl(topRight);
			vec4 temp = vec4(0, 0, 0, 1) * selectedEditable->getGlobalMatrix() * cam.getViewMatrix() * cam.getPerspective(fabsf((topRight.x-bottomLeft.x)/(topRight.y-bottomLeft.y)));
			temp = temp / temp.w;
			operationHelper21 = vec2(bottomLeft.x, topRight.y) + vec2((topRight.x - bottomLeft.x) * (0.5f * temp.x + 0.5f), (bottomLeft.y-topRight.y) * (0.5f * temp.y + 0.5f));//position of object on screen
			operationHelper22 = vec2(pX, pY);
			operationHelper31 = selectedEditable->getScale();

			operationCenterFromCamera = 1.732f;//it is the distance that is used when comparing in calcMouseSens
		} while (0);
		break;

	case SCALE_VERTEX:
		//operationHelper21: the screen space position of the center of the selected vertices
		//operationHelper22: the mouse position in screen space at the start of the operation
		//operationHelper31: the position of the center of the selected vertices in model space (so that it doesn't need to be calculated every time)

		do {
			Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);
			bottomLeft = System::convertScreenToGl(bottomLeft);
			topRight = System::convertScreenToGl(topRight);

			//the center of the selected vertices
			operationHelper31 = vec3();
			for (int i = 0; i < operationRollbackVertex.size(); i++)
				operationHelper31 = operationHelper31 + operationRollbackVertex[i].data.position;
			operationHelper31 = operationHelper31 / (float)operationRollbackVertex.size();

			//mouse positions
			vec4 temp = vec4(operationHelper31, 1) * selectedEditable->getGlobalMatrix() * cam.getViewMatrix() * cam.getPerspective(fabsf((topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y)));
			temp = temp / temp.w;
			operationHelper21 = vec2(bottomLeft.x, topRight.y) + vec2((topRight.x - bottomLeft.x) * (0.5f * temp.x + 0.5f), (bottomLeft.y - topRight.y) * (0.5f * temp.y + 0.5f));//position of object on screen
			operationHelper22 = vec2(pX, pY);

			//operation distance
			operationCenterFromCamera = length(cam.getPosition() - operationHelper31);
		} while (0);
		break;

	case SCALE_VERTEX_UV:
		//operationHelper21: the screen space position of the center of the selected vertices
		//operationHelper22: the mouse position in screen space at the start of the operation
		//operationHelper31: the position of the center of the selected vertices in model space (so that it doesn't need to be calculated every time)

		do {
			Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight);
			bottomLeft = System::convertScreenToGl(bottomLeft);
			topRight = System::convertScreenToGl(topRight);

			float aspectXY = (topRight.x - bottomLeft.x) / (topRight.y-bottomLeft.y);
			mat4 vp = cam2D.getViewMatrix() * OrthoMatrix(-0.5f * aspectXY * cam2Dzoom, 0.5f * aspectXY * cam2Dzoom, -0.5f * cam2Dzoom, 0.5f * cam2Dzoom, 0, 10.0f);
			
			//the center of the selected vertices
			operationHelper31 = vec3();
			for (int i = 0; i < operationRollbackVertex.size(); i++)
				operationHelper31 = operationHelper31 + operationRollbackVertex[i].data.uv;
			operationHelper31 = operationHelper31 / (float)operationRollbackVertex.size();
	

			//mouse positions
			vec4 temp = vec4(operationHelper31, 1) * vp;
			operationHelper21 = bottomLeft + vec2((topRight.x - bottomLeft.x) * (0.5f * temp.x / temp.w + 0.5f), (topRight.y - bottomLeft.y) * (0.5f * temp.y / temp.w + 0.5f));//position of object on screen
			operationHelper22 = vec2(pX, pY);
		} while (0);
		break;

	case ROTATE_OBJECT:
		//operationHelper21: the overall mouse movement

		operationCenterFromCamera = length(selectedEditable->getPosition() - cam.getPosition());
		operationHelper21 = vec2();
		break;

	case ROTATE_VERTEX:
		//operationHelper21: the overall mouse movement
		//operationHelper31: the center of rotation

		do {
			vec3 temp = vec3();
			const std::vector<VertexData>& vertices = selectedEditable->getVertices();
			for (auto& i = selectedVertexIDs.begin(); i < selectedVertexIDs.end(); i++)
				temp = temp + vertices[*i].position;
			if (selectedVertexIDs.size() > 0)
				temp = temp / selectedVertexIDs.size();

			operationCenterFromCamera = length(temp-cam.getPosition());
			operationHelper31 = temp;

			operationHelper21 = vec2();
		} while (0);
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

	operationRollbackObject.clear();
	operationRollbackVertex.clear();
	currentOperation = Operation::NONE;
	currentOperationDirection = OperationDirection::DIR_ALL;
}

void processOperation(int dX, int dY)
{
	int pX, pY;
	System::getMousePosition(&pX, &pY);

	VertexData vd;
	vec2 delta2;
	vec3 delta3;
	vec2 bottomLeft, topRight;
	vec2 mouseNDC;
	switch (currentOperation)
	{
	case Operation::MOVE_VERTEX_UV:
		Layout::getLayoutBounds(Layout::UV, &bottomLeft, &topRight);
		delta2= calculateMouseSensitivity2D(bottomLeft, topRight)*vec2(dX, -dY);
		operationHelper21 = operationHelper21 + delta2;

		for (int i = 0; i < selectedVertexIDs.size(); i++)
		{
			vd = selectedEditable->getVertices()[selectedVertexIDs[i]];
			vd.uv = vd.uv + delta2;
			selectedEditable->setVertexData(selectedVertexIDs[i], vd);
		}
		break;

	case Operation::MOVE_VERTEX:
		Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);
		delta2 = 0.5f*calculateMouseSensitivity3D(bottomLeft, topRight);
		operationHelper31 = operationHelper31 + delta2.x * dX * cam.getRight() - delta2.y * dY * cam.getUp();
		delta3 = operationHelper31;
		
		switch (currentOperationDirection)
		{
			case OD::DIR_X: delta3[1] = delta3[2] = 0; break;
			case OD::DIR_Y: delta3[0] = delta3[2] = 0; break;
			case OD::DIR_Z: delta3[0] = delta3[1] = 0; break;
		}

		for (int i = 0; i < selectedVertexIDs.size(); i++)
		{
			vd = operationRollbackVertex[i].data;
			vd.position = vd.position + delta3;

			selectedEditable->setVertexData(selectedVertexIDs[i], vd);
		}
		break;

	case Operation::MOVE_OBJECT:
		Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight);
		delta2 = 0.5f * calculateMouseSensitivity3D(bottomLeft, topRight);
		operationHelper31 = operationHelper31 + delta2.x * dX * cam.getRight() - delta2.y * dY * cam.getUp();
		delta3 = operationHelper31;

		switch (currentOperationDirection)
		{
			case OD::DIR_X: delta3[1] = delta3[2] = 0; break;
			case OD::DIR_Y: delta3[0] = delta3[2] = 0; break;
			case OD::DIR_Z: delta3[0] = delta3[1] = 0; break;
		}

		selectedEditable->setPosition(operationRollbackObject[0].position + delta3);
		selectedEditable->recalculateGlobalMatrix();
		break;

	case Operation::SCALE_OBJECT:
		do {
			float ratio = length(vec2(pX, pY) - operationHelper21) / length(operationHelper22 - operationHelper21);
			if (dot(vec2(pX, pY) - operationHelper21, operationHelper22 - operationHelper21) < 0)
				ratio *= -1;
			vec3 ratioVec = vec3(ratio, ratio, ratio);

			switch (currentOperationDirection)
			{
			case OD::DIR_X: ratioVec[1] = ratioVec[2] = 1; break;
			case OD::DIR_Y: ratioVec[0] = ratioVec[2] = 1; break;
			case OD::DIR_Z: ratioVec[0] = ratioVec[1] = 1; break;
			}

			selectedEditable->setScale(ratioVec * operationHelper31);
			selectedEditable->recalculateGlobalMatrix();
		} while (0);
		break;

	case Operation::SCALE_VERTEX:
		do {
			float ratio = length(vec2(pX, pY) - operationHelper21) / length(operationHelper22 - operationHelper21);
			if (dot(vec2(pX, pY) - operationHelper21, operationHelper22 - operationHelper21) < 0)
				ratio *= -1;
			vec3 ratioVec = vec3(ratio, ratio, ratio);

			switch (currentOperationDirection)
			{
			case OD::DIR_X: ratioVec[1] = ratioVec[2] = 1; break;
			case OD::DIR_Y: ratioVec[0] = ratioVec[2] = 1; break;
			case OD::DIR_Z: ratioVec[0] = ratioVec[1] = 1; break;
			}

			for (int i = 0; i < operationRollbackVertex.size(); i++)
			{
				vec3 newVertexPos = ratioVec *(operationRollbackVertex[i].data.position-operationHelper31)+operationHelper31;
				selectedEditable->setVertexData(
					operationRollbackVertex[i].vertexID,
					VertexData(newVertexPos, operationRollbackVertex[i].data.uv)
				);
			}
		} while (0);
		break;

	case Operation::SCALE_VERTEX_UV:
		do {
			float ratio = length(vec2(pX, pY) - operationHelper21) / length(operationHelper22 - operationHelper21);
			if (dot(vec2(pX, pY) - operationHelper21, operationHelper22 - operationHelper21) < 0)
				ratio *= -1;

			vec2 center = vec2(operationHelper31.x, operationHelper31.y);
			for (int i = 0; i < operationRollbackVertex.size(); i++)
			{
				vec2 newVertexUv = ratio * (operationRollbackVertex[i].data.uv - center) + center;
				selectedEditable->setVertexData(
					operationRollbackVertex[i].vertexID,
					VertexData(operationRollbackVertex[i].data.position,newVertexUv)
				);
			}
		} while (0);
		break;

	case Operation::ROTATE_OBJECT:
		do {
			operationHelper21 = operationHelper21 + vec2(dX, dY);

			vec3 rotAxis;
			float angle=1;

			switch (currentOperationDirection)
			{
			case OD::DIR_X: 
				do {
					rotAxis = vec3(1, 0, 0);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.x = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = 0.01f * length(temp);
					else
						angle = -0.01f * length(temp);
				} while (0);
				break;
			case OD::DIR_Y: 
				do {
					rotAxis = vec3(0, 1, 0);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.y = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = -0.01f*length(temp);
					else
						angle = 0.01f*length(temp);
				} while (0);
				break;
			case OD::DIR_Z: 
				do {
					rotAxis = vec3(0, 0, 1);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.z = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = 0.01f * length(temp);
					else
						angle = -0.01f * length(temp);
				} while (0);
				break;
			default:
				rotAxis = normalize(operationHelper21.x * cam.getUp() + operationHelper21.y * cam.getRight());
				angle = 0.01f * length(operationHelper21);
				break;
			}

			quat temp = Quaternion(angle, rotAxis);

			selectedEditable->setRotation(temp * operationRollbackObject[0].rotation);
			selectedEditable->recalculateGlobalMatrix();
		} while (0);
		break;

	case Operation::ROTATE_VERTEX:
		do {
			operationHelper21 = operationHelper21 + vec2(dX, dY);

			vec3 rotAxis;
			float angle = 1;

			switch (currentOperationDirection)
			{
			case OD::DIR_X:
				do {
					rotAxis = vec3(1, 0, 0);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.x = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = 0.01f * length(temp);
					else
						angle = -0.01f * length(temp);
				} while (0);
				break;
			case OD::DIR_Y:
				do {
					rotAxis = vec3(0, 1, 0);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.y = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = -0.01f * length(temp);
					else
						angle = 0.01f * length(temp);
				} while (0);
				break;
			case OD::DIR_Z:
				do {
					rotAxis = vec3(0, 0, 1);
					vec3 temp = operationHelper21.y * cam.getUp() + operationHelper21.x * cam.getRight();
					temp.z = 0;
					temp = cross(rotAxis, temp);
					if (dot(temp, cam.getDirection()) < 0)
						angle = 0.01f * length(temp);
					else
						angle = -0.01f * length(temp);
				} while (0);
				break;
			default:
				rotAxis = normalize(operationHelper21.x * cam.getUp() + operationHelper21.y * cam.getRight());
				angle = 0.01f * length(operationHelper21);
				break;
			}

			quat q = Quaternion(angle, rotAxis);
			for (int i = 0; i < operationRollbackVertex.size(); i++)
			{
#define o operationRollbackVertex[i]
				VertexData cucc=o.data;
				cucc.position = Quaternion::rotateVector(o.data.position - operationHelper31, q) + operationHelper31;
				selectedEditable->setVertexData(o.vertexID, cucc);
#undef o
			}
		} while (0);
		break;
	}
}

vec3 calculateOperationCenter()
{
	vec3 vissza=vec3();

	switch (currentOperation)
	{
		case Operation::MOVE_OBJECT:
			for (int i = 0; i < operationRollbackObject.size(); i++)
				vissza = vissza + operationRollbackObject[i].position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackObject.size();
			break;

		case Operation::MOVE_VERTEX:
			for (int i = 0; i < operationRollbackVertex.size(); i++)
				vissza = vissza + operationRollbackVertex[i].data.position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackVertex.size();
			break;

		case Operation::SCALE_OBJECT:
			for (int i = 0; i < operationRollbackObject.size(); i++)
				vissza = vissza + operationRollbackObject[i].position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackObject.size();
			break;

		case Operation::SCALE_VERTEX:
			for (int i = 0; i < operationRollbackVertex.size(); i++)
				vissza = vissza + operationRollbackVertex[i].data.position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackVertex.size();
			break;

		case Operation::ROTATE_OBJECT:
			for (int i = 0; i < operationRollbackObject.size(); i++)
				vissza = vissza + operationRollbackObject[i].position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackObject.size();
			break;

		case Operation::ROTATE_VERTEX:
			for (int i = 0; i < operationRollbackVertex.size(); i++)
				vissza = vissza + operationRollbackVertex[i].data.position;
			if (operationRollbackVertex.size() > 0)
				vissza = vissza / operationRollbackVertex.size();
			break;
	}

	return vissza;
}

vec2 calculateMouseSensitivity3D(const vec2& bottomLeft, const vec2& topRight)//the fov of the 3D camera is counted
{
	static const vec2 MOUSE_SENSITIVITY_AT_1080P_60FOV = vec2(0.0034f, 0.0034f);

	//calculate the screen ratio
	vec2 ratio = vec2(1920.0f/ fabsf(topRight.x - bottomLeft.x), 1080.0f/fabsf(topRight.y - bottomLeft.y));

	//calculate the aspect ratio
	ratio.x *= fabsf((topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y)) * 0.5625f;//0.5625 is the 1/aspectXY @ 1080p

	//calculate the fov ratio ( 0.5 = sin(60/2) )
	ratio = ratio * (0.5f / sinf(DEG2RAD * 0.5f * cam.getFov()));

	//distance from camera
	ratio = ratio * (operationCenterFromCamera / 1.732f);

	return MOUSE_SENSITIVITY_AT_1080P_60FOV * ratio;
}

vec2 calculateMouseSensitivity2D(const vec2& bottomLeft, const vec2& topRight)
{
	static const vec2 MOUSE_SENSITIVITY_AT_1080P_1ZOOM = vec2(0.00185f, 0.00185f);

	//calculate the screen ratio
	vec2 ratio = vec2(1920.0f / fabsf(topRight.x - bottomLeft.x), 1080.0f / fabsf(topRight.y - bottomLeft.y));

	//calculate the aspect ratio
	ratio.x *= fabsf((topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y)) * 0.5625f;//0.5625 is the 1/aspectXY @ 1080p

	//zoom
	ratio = cam2Dzoom * ratio;

	return MOUSE_SENSITIVITY_AT_1080P_1ZOOM * ratio;
}


//imgui stuff
void closeAllLocalLists()
{
	ObjectLocalList::close();
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

		if (Layout::getLayoutBounds(Layout::OBJECT, &bottomLeft, &topRight))
			ObjectLocalList::render(bottomLeft, topRight);

		Layout::renderImGUI();
	} while (0);


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}