#include "editable.h"

#include <GL/glew.h>
#include <string.h>

#include "../ImGui/imgui.h"
#include "../System/system.h"
#include "../ui/header/header.h"

static int editablesCreated = 0;

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

Editable::Editable(Preset preset)
{

}

Editable::~Editable()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
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


//static part

std::vector<Editable*> Editable::edibles = std::vector<Editable*>();


void Editable::initialize()
{

}

void Editable::deinitialize()
{
	for (int i = 0; i < Editable::edibles.size(); i++)
		delete edibles[i];
	edibles.clear();
}



Editable* Editable::add(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
{
	Editable* logus = new Editable(vertices, indices, vertexCount, indexCount);
	Editable::edibles.push_back(logus);
	return logus;
}

Editable* Editable::add(Editable::Preset preset)
{
	Editable* logus = new Editable(preset);
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
