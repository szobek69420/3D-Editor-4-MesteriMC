#include "editable.h"

#include <GL/glew.h>
#include <string.h>

#include "../framework.h"

#include "../ImGui/imgui.h"
#include "../System/system.h"
#include "../ui/header/header.h"
#include "../Camera/camera.h"

static int editablesCreated = 0;

static GPUProgram program3D, program2D, program2DRectangle;
static unsigned int rectangleVAO, rectangleVBO;
static float rectangleVBOContent[] = { -1,-1,1,-1,1,1,1,1,-1,1,-1,-1 };

extern Editable* selectedEditable;

Editable::Editable(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
{
	editablesCreated++;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexData), vertices, GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(3*sizeof(float)));

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	this->vertices.assign(vertices, vertices+vertexCount);
	this->indices.assign(indices, indices + indexCount);

	localPosition = vec3(0);
	localScale = vec3(1);
	localRotation = vec3(0);
	globalModelMatrix = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	parent = NULL;

	sprintf_s(name, 100, "object_%d", editablesCreated);
}

Editable::~Editable()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	if (this->albedo)
		glDeleteTextures(1, &this->albedo);
}

mat4 Editable::calculateLocalMatrix()
{
	mat4 local = mat4(localScale.x, 0, 0, 0, 0, localScale.y, 0, 0, 0, 0, localScale.z, 0, localPosition.x, localPosition.y, localPosition.z, 1);
	//add the rotation later
	return local;
}

void Editable::recalculateGlobalMatrix(const mat4& parentGlobalModel)
{
	this->globalModelMatrix = calculateLocalMatrix() * parentGlobalModel;
	for (int i = 0; i < this->children.size(); i++)
		this->children[i]->recalculateGlobalMatrix(this->globalModelMatrix);
}

void Editable::setParent(Editable* parent)
{
	if (this->parent != NULL && parent != this->parent)//remove from the last parent
	{
		for (int i = 0; i < this->parent->children.size(); i++)
		{
			if (this->parent->children[i] == this)
			{
				this->parent->children.erase(this->parent->children.begin() + i);
				break;
			}
		}
	}

	if (parent!=NULL&&parent != this->parent)//add to the new parent
	{
		parent->children.push_back(this);
	}

	this->parent = parent;
}

void Editable::setName(const char* name)
{
	strcpy_s(this->name, 100, name);
}

void Editable::setAlbedo(unsigned int texture)
{
	if (this->albedo != 0)
		glDeleteTextures(1, &this->albedo);
	this->albedo = texture;
}
unsigned int Editable::getAlbedo()
{
	return this->albedo;
}


//static part
static VertexData presetVertices_Cube[] = {
	VertexData(vec3(-1,-1,-1),vec2(0,0)),
	VertexData(vec3(1,-1,-1),vec2(0,0)),
	VertexData(vec3(1,-1,1),vec2(0,0)),
	VertexData(vec3(-1,-1,1),vec2(0,0)),
	VertexData(vec3(-1,1,-1),vec2(0,0)),
	VertexData(vec3(1,1,-1),vec2(0,0)),
	VertexData(vec3(1,1,1),vec2(0,0)),
	VertexData(vec3(-1,1,1),vec2(0,0))
};

static unsigned int presetIndices_Cube[] = {
	0,4,1,	4,5,1,
	1,5,2,	5,6,2,
	2,6,3,	6,7,3,
	3,7,0,	7,4,0,
	0,1,2,	2,3,0,
	4,6,5,	6,4,7
};

std::vector<Editable*> Editable::edibles = std::vector<Editable*>();


void Editable::initialize()
{
	program3D.createFromFile(
		"./assets/shaders/render3D/shader_3d.vag",
		"./assets/shaders/render3D/shader_3d.fag",
		"fragColour",
		nullptr);

	program2D.createFromFile("./assets/shaders/render2D/shader_2d.vag",
		"./assets/shaders/render2D/shader_2d.fag",
		"fragColour",
		nullptr);

	program2DRectangle.createFromFile("./assets/shaders/render2D/shader_2d_rectangle.vag",
		"./assets/shaders/render2D/shader_2d_rectangle.fag",
		"fragColour",
		nullptr);

	glGenVertexArrays(1, &rectangleVAO);
	glBindVertexArray(rectangleVAO);
	glGenBuffers(1, &rectangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVBOContent), rectangleVBOContent, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glBindVertexArray(0);
}

void Editable::deinitialize()
{
	for (int i = 0; i < Editable::edibles.size(); i++)
		delete edibles[i];
	edibles.clear();

	glDeleteVertexArrays(1, &rectangleVAO);
	glDeleteBuffers(1, &rectangleVBO);
}



Editable* Editable::add(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
{
	Editable* logus = new Editable(vertices, indices, vertexCount, indexCount);
	Editable::edibles.push_back(logus);
	return logus;
}

Editable* Editable::add(Editable::Preset preset)
{
	Editable* logus = NULL;
	switch (preset)
	{
	case Editable::Preset::CUBE:
		logus = new Editable(presetVertices_Cube, presetIndices_Cube, 8, 36);
		break;

	default:
		return NULL;
	}

	if(logus!=NULL)
		Editable::edibles.push_back(logus);
	return logus;
}

void Editable::remove(Editable* edible)
{
	for (int i = 0; i < edible->children.size(); i++)
		edible->children[i]->parent = edible->parent;

	for (int i = 0; i < edibles.size(); i++)
	{
		if (edibles[i] == edible)
		{
			edibles.erase(edibles.begin() + i);
			break;
		}
	}

	delete edible;
}

void Editable::removeWithChildren(Editable* edible)
{
	for (int i = 0; i < edible->children.size(); i++)
		Editable::removeWithChildren(edible->children[i]);
	Editable::remove(edible);
}

void Editable::renderHierarchyItem(Editable* edible)
{
	ImGui::Text(edible->name);
	ImGui::Indent(10);
	for (int i = 0; i < edible->children.size(); i++)
		Editable::renderHierarchyItem(edible->children[i]);
	ImGui::Unindent(10);
}

void Editable::renderHierarchy()
{
	static int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui::SetNextWindowPos(ImVec2(windowWidth - 200, Header::HeightInPixels));
	ImGui::SetNextWindowSize(ImVec2(200, 300));
	ImGui::Begin("hierarchy", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	for (int i = 0; i < edibles.size(); i++)
	{
		if (edibles[i]->parent != NULL)
			continue;
		Editable::renderHierarchyItem(edibles[i]);
	}

	ImGui::End();
}


void Editable::render3D(const Camera& camera, vec2 bottomLeft, vec2 topRight)
{
	mat4 projection = PerspectiveMatrix(60, (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y), 0.01f, 100.0f);

	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glPointSize(5);
	glLineWidth(2);

	program3D.Use();
	program3D.setUniform(projection, "projection");
	program3D.setUniform(camera.getViewMatrix(), "view");
	program3D.setUniform(0, "tex");
	program3D.setUniform(vec3(0), "colour");

	for (int i = 0; i < Editable::edibles.size(); i++)
	{
		program3D.setUniform(Editable::edibles[i]->globalModelMatrix, "model");
		glBindVertexArray(Editable::edibles[i]->vao);

		if (/*texture is loaded*/0)
			program3D.setUniform(69, "isSampled");
		else
		{
			program3D.setUniform(0, "isSampled");
			program3D.setUniform(vec3(0.5f,0.5f,0.5f), "colour");
		}

		glDrawElements(GL_TRIANGLES, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);


		program3D.setUniform(0, "isSampled");

		program3D.setUniform(vec3(0, 0, 1), "colour");
		glPolygonMode(GL_FRONT, GL_LINE);
		glDrawElements(GL_TRIANGLES, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT, GL_FILL);

		program3D.setUniform(vec3(1, 0.85f, 0), "colour");
		glDrawElements(GL_POINTS, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);
	}

	glBindVertexArray(0);
	glUseProgram(0);

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
}

void Editable::render2D(const Camera& cum, vec2 bottomLeft, vec2 topRight)
{
	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);

	mat4 projection = PerspectiveMatrix(60, (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y), 0.01f, 100.0f);
	
	//render texture
	program2DRectangle.Use();
	program2DRectangle.setUniform(0, "tex");
	if (selectedEditable == NULL || selectedEditable->getAlbedo() == 0)
		program2DRectangle.setUniform(0, "isSampled");
	else
	{
		program2DRectangle.setUniform(69, "isSampled");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, selectedEditable->getAlbedo());
	}

	program2DRectangle.setUniform(cum.getViewMatrix(), "view");
	program2DRectangle.setUniform(projection, "projection");

	glBindVertexArray(rectangleVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
