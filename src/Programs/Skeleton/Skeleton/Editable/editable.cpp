#include "editable.h"

#include <GL/glew.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include "../framework.h"

#include "../ImGui/imgui.h"
#include "../System/system.h"
#include "../ui/header/header.h"
#include "../Camera/camera.h"

#include "../TextureLoader/texture_loader.h"

class SerializableEditable {
public:
	unsigned int id;
	char name[EDIBLE_NAME_MAX_LENGTH];
	vec3 localPosition;
	vec3 localScale;
	vec3 localRotation;

	unsigned int parentId;//the id of the parent (0 is fatherless)
	std::vector<unsigned int> childId;//the ids of the children

	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;

	char albedoPath[EDIBLE_PATH_MAX_LENGTH];

	SerializableEditable()
	{
		//does nothing
	}

	SerializableEditable(const Editable* edible)
	{
		this->id = edible->id;
		memcpy(this->name, edible->name, EDIBLE_NAME_MAX_LENGTH);
		this->localPosition = edible->localPosition;
		this->localScale = edible->localScale;
		this->localRotation = edible->localRotation;
		this->parentId = edible->parent == NULL ? 0 : edible->parent->id;
		for (int i = 0; i < edible->children.size(); i++)
			this->childId.push_back(edible->children[i]->id);
		this->vertices.assign(edible->vertices.begin(), edible->vertices.end());
		this->indices.assign(edible->indices.begin(), edible->indices.end());
		
		if (edible->albedo != 0)
			memcpy(this->albedoPath, edible->albedoPath, EDIBLE_PATH_MAX_LENGTH);
		else
			strcpy(this->albedoPath, "none");
	}
};


static int currentEditableId = 1;//0 means no object

static GPUProgram program3D, program3DUnlit, program3DNormal;
static GPUProgram program2D, program2DRectangle;

static unsigned int rectangleVAO, rectangleVBO;
static float rectangleVBOContent[] = { 0,0,1,0,1,1,1,1,0,1,0,0 };

extern Editable* selectedEditable;
extern std::vector<unsigned int> selectedVertexIDs;

Editable::Editable(const VertexData* vertices, const unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
{
	id= currentEditableId++;

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
	localScale = vec3(1,1,1);
	localRotation = vec3(0);
	globalModelMatrix = calculateLocalMatrix();

	parent = NULL;

	sprintf_s(name, 100, "object_%d", currentEditableId);
	strcpy(albedoPath, "");
}

Editable::~Editable()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	if (this->albedo)
		Editable::releaseTexture(this->albedo);
}

mat4 Editable::calculateLocalMatrix()
{
	mat4 local = ScaleMatrix(localScale)*TranslateMatrix(localPosition);
	//add the rotation later
	return local;
}

void Editable::refreshVertexBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Editable::refreshIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Editable::recalculateGlobalMatrix()
{
	if(parent==NULL)
		this->globalModelMatrix = calculateLocalMatrix();
	else
		this->globalModelMatrix = calculateLocalMatrix()*parent->globalModelMatrix;

	for (int i = 0; i < this->children.size(); i++)
		this->children[i]->recalculateGlobalMatrix();
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


const std::vector<VertexData>& Editable::getVertices() const
{
	return this->vertices;
}

void Editable::setVertexData(unsigned int vertexID, VertexData data)
{
	this->vertices[vertexID] = data;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, vertexID * sizeof(VertexData), sizeof(VertexData), &data);
}


const std::vector<unsigned int>& Editable::getIndices() const
{
	return this->indices;
}


mat4 Editable::getGlobalMatrix()
{
	return this->globalModelMatrix;
}


void Editable::setAlbedo(const char* albedoPath)
{
	if (this->albedo != 0)
		Editable::releaseTexture(this->albedo);

	this->albedo = Editable::importTexture(albedoPath);
	
	//trim path
	if (strlen(albedoPath) > 199)
		strcpy(this->albedoPath, albedoPath + strlen(albedoPath) - 199);
	else
		strcpy(this->albedoPath, albedoPath);
}
unsigned int Editable::getAlbedo()
{
	return this->albedo;
}

const char* Editable::getAlbedoPath()
{
	return this->albedoPath;
}

vec3 Editable::getPosition()
{
	return this->localPosition;
}
void Editable::setPosition(const vec3& position)
{
	this->localPosition = position;
}

vec3 Editable::getRotation()
{
	return this->localRotation;
}
void Editable::setRotation(const vec3& rotation)
{
	this->localRotation = rotation;
}

vec3 Editable::getScale()
{
	return this->localScale;
}
void Editable::setScale(const vec3& scale)
{
	this->localScale = scale;
}


void Editable::addVertex(vec3 pos, vec2 uv)
{
	vertices.push_back(VertexData(pos, uv));
	refreshVertexBuffer();
}

void Editable::removeVertex(unsigned int vertexID)
{
	vertices.erase(vertices.begin() + vertexID);

	for (int i = 0; i < indices.size(); i += 3)
	{
		if (indices[i] == vertexID || indices[i + 1] == vertexID || indices[i + 2] == vertexID)//the face should be removed
		{
			indices.erase(indices.begin() + i, indices.begin() + i + 3);
			i -= 3;
		}
		else //check if there are any indices that should be altered
		{
			if (indices[i] > vertexID)
				indices[i]--;
			if (indices[i + 1] > vertexID)
				indices[i + 1]--;
			if (indices[i + 2] > vertexID)
				indices[i + 2]--;
		}
	}

	refreshVertexBuffer();
	refreshIndexBuffer();
}

void Editable::addFace(unsigned int vertexID0, unsigned int vertexID1, unsigned int vertexID2)
{
	indices.push_back(vertexID0);
	indices.push_back(vertexID1);
	indices.push_back(vertexID2);

	refreshIndexBuffer();
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
std::vector<EditableTexture> Editable::textures = std::vector<EditableTexture>();


void Editable::initialize()
{
	program3D.createFromFile(
		"./assets/shaders/render3D/shader_3d.vag",
		"./assets/shaders/render3D/shader_3d.fag",
		"fragColour",
		"./assets/shaders/render3D/shader_3d.gag");

	program3DUnlit.createFromFile(
		"./assets/shaders/render3D/shader_3d_unlit.vag",
		"./assets/shaders/render3D/shader_3d_unlit.fag",
		"fragColour",
		nullptr);

	program3DNormal.createFromFile(
		"./assets/shaders/render3D/normal/shader_3d_normal.vag",
		"./assets/shaders/render3D/normal/shader_3d_normal.fag",
		"fragColour",
		"./assets/shaders/render3D/normal/shader_3d_normal.gag"
		);


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

void Editable::saveAs(const char* filePath)
{
	FILE* file = fopen(filePath, "w");
	if (file == NULL)
	{
		printf("save file could not be created/opened\n");
		return;
	}

	fprintf(file, "version: 0.69\n");
	fprintf(file, "count: %d\n", Editable::edibles.size());
	for (int i = 0; i < Editable::edibles.size(); i++)
	{
		SerializableEditable se(Editable::edibles[i]);
		Editable::printEditableToFile(&se, file);
	}

	fclose(file);
}

void Editable::importFrom(const char* filePath)
{
	FILE* file = fopen(filePath, "r");
	if (file == NULL)
	{
		printf("save file could not be opened\n");
		return;
	}

	//clearing previous state
	for (int i = Editable::edibles.size() - 1; i >= 0; i--)
		Editable::remove(Editable::edibles[i]);

	//reading stuff
	int major, minor;
	fscanf_s(file, "version: %d.%d\n", &major, &minor);

	int count;
	fscanf_s(file, "count: %d\n", &count);

	std::vector<SerializableEditable> se;
	for (int i = 0; i < count; i++)
		se.push_back(Editable::readEditableFromFile(file));

	fclose(file);

	//building scene
	for (int i = 0; i < se.size(); i++)
		Editable::serializableToEditable(&se[i]);

	for (int i = 0; i < Editable::edibles.size(); i++)//mindegyiknek megkeresem a szulojet
	{
		if (se[i].parentId == 0)//fatherless
		{
			Editable::edibles[i]->parent = NULL;
			continue;
		}

		for (int j = 0; j < Editable::edibles.size(); j++)//search for parent
		{
			if (i == j)
				continue;

			if (se[i].parentId == Editable::edibles[j]->id)
			{
				Editable::edibles[i]->parent = Editable::edibles[j];//adding as parent
				Editable::edibles[j]->children.push_back(Editable::edibles[i]);//adding as child
				break;
			}
		}
	}

	//calculate matrices
	for (int i = 0; i < Editable::edibles.size(); i++)
	{
		if (Editable::edibles[i]->parent == NULL)//only calculate model matrix if fatherless, as it is a recursive operation
			Editable::edibles[i]->recalculateGlobalMatrix();
	}

	//set id counter
	for (int i = 0; i < Editable::edibles.size(); i++)
		if (currentEditableId < Editable::edibles[i]->id)
			currentEditableId = Editable::edibles[i]->id;
	currentEditableId++;
}

void Editable::printEditableToFile(const SerializableEditable* edible, FILE* file)
{
	fprintf(file, "id: %d\n", edible->id);

	if (strnlen_s(edible->name, EDIBLE_NAME_MAX_LENGTH) == 0)
		fprintf(file, "name: sus\n");
	else
		fprintf(file, "name: %s\n", edible->name);

	fprintf(file, "pos: %.5f, %.5f, %.5f\n", edible->localPosition.x, edible->localPosition.y, edible->localPosition.z);
	fprintf(file, "scale: %.5f, %.5f, %.5f\n", edible->localScale.x, edible->localScale.y, edible->localScale.z);
	fprintf(file, "rot: %.5f, %.5f, %.5f\n", edible->localRotation.x, edible->localRotation.y, edible->localRotation.z);

	fprintf(file, "parent: %d\n", edible->parentId);

	fprintf(file, "child count: %d\n", edible->childId.size());
	for (int i = 0; i < edible->childId.size(); i++)
		fprintf(file, "%d\n", edible->childId[i]);

	fprintf(file, "vertex count: %d\n", edible->vertices.size());
	for (int i = 0; i < edible->vertices.size(); i++)
		fprintf(
			file,
			"%.5f, %.5f, %.5f, %.5f, %.5f\n",
			edible->vertices[i].position.x,
			edible->vertices[i].position.y,
			edible->vertices[i].position.z,
			edible->vertices[i].uv.x,
			edible->vertices[i].uv.y
		);
	fprintf(file, "index count: %d\n", edible->indices.size());
	for (int i = 0; i < edible->indices.size(); i++)
		fprintf(file, "%d\n", edible->indices[i]);

	fprintf(file, "albedo: %s\n", edible->albedoPath);
}

SerializableEditable Editable::readEditableFromFile(FILE* file)
{
	SerializableEditable se;

	unsigned int count;

	fscanf_s(file, "id: %d\n", &se.id);
	fscanf_s(file, "name: %s\n", se.name, EDIBLE_NAME_MAX_LENGTH);

	fscanf_s(file, "pos: %f, %f, %f\n", &se.localPosition.x, &se.localPosition.y, &se.localPosition.z);
	fscanf_s(file, "scale: %f, %f, %f\n", &se.localScale.x, &se.localScale.y, &se.localScale.z);
	fscanf_s(file, "rot: %f, %f, %f\n", &se.localRotation.x, &se.localRotation.y, &se.localRotation.z);
	
	fscanf_s(file, "parent: %d\n", &se.parentId);
	fscanf_s(file, "child count: %d\n", &count);
	for (unsigned int i = 0; i < count; i++)
	{
		se.childId.push_back(69);
		fscanf_s(file, "%d\n", &se.childId[i]);
	}

	fscanf_s(file, "vertex count: %d\n", &count);
	for (unsigned int i = 0; i < count; i++)
	{
		VertexData vd;
		fscanf_s(
			file,
			"%f, %f, %f, %f, %f\n",
			&vd.position.x,
			&vd.position.y,
			&vd.position.z,
			&vd.uv.x,
			&vd.uv.y
		);
		se.vertices.push_back(vd);
	}
	fscanf_s(file, "index count: %d\n", &count);
	for (int i = 0; i < count; i++)
	{
		se.indices.push_back(69);
		fscanf_s(file, "%d\n", &se.indices[i]);
	}

	fscanf_s(file, "albedo: %s\n", se.albedoPath, EDIBLE_PATH_MAX_LENGTH);

	return se;
}

Editable* Editable::serializableToEditable(const SerializableEditable* se)
{
	Editable* edible = Editable::add(se->vertices.data(), se->indices.data(), se->vertices.size(), se->indices.size());

	edible->id = se->id;
	memcpy(edible->name, se->name, EDIBLE_NAME_MAX_LENGTH);

	edible->localPosition = se->localPosition;
	edible->localScale = se->localScale;
	edible->localRotation = se->localRotation;
	
	if (strncmp("none", se->albedoPath, EDIBLE_PATH_MAX_LENGTH) == 0)
		edible->albedo = 0;
	else
		edible->setAlbedo(se->albedoPath);

	return edible;
}



Editable* Editable::add(const VertexData* vertices, const unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
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

Editable* Editable::clone(Editable* edible)
{
	Editable* edible2 = Editable::add(edible->vertices.data(), edible->indices.data(), edible->vertices.size(), edible->indices.size());
	
	//copy name
	strcpy(edible2->name, "copy of ");
	strcpy_s(edible2->name+8, sizeof(edible2->name) - 8, edible->name);

	//copy orientation data
	edible2->localPosition = edible->localPosition;
	edible2->localRotation = edible->localRotation;
	edible2->localScale = edible->localScale;
	edible2->globalModelMatrix = edible->globalModelMatrix;

	//copy parent
	edible2->parent = edible->parent;

	//copy texture
	edible2->albedo = edible->albedo;
	Editable::incrementTextureReference(edible2->albedo);
	strcpy(edible2->albedoPath, edible->albedoPath);

	return edible2;
}

std::vector<Editable*> Editable::getEditables()
{
	std::vector<Editable*> walter;
	walter.assign(Editable::edibles.begin(), Editable::edibles.end());
	return walter;
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


void Editable::render3D(const Camera& camera, vec2 bottomLeft, vec2 topRight, int showVertices, int showNormals)
{
	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glPointSize(5);
	glLineWidth(2);

	mat4 projection = camera.getPerspective((topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y));

	program3D.Use();
	program3D.setUniform(projection, "projection");
	program3D.setUniform(camera.getViewMatrix(), "view");
	program3D.setUniform(0, "tex");
	program3D.setUniform(vec3(0), "colour");
	program3D.setUniform(normalize(vec3(0.6f, 1.0f, 0.8f)), "lightDirNormalized");

	program3DUnlit.Use();
	program3DUnlit.setUniform(projection, "projection");
	program3DUnlit.setUniform(camera.getViewMatrix(), "view");
	program3DUnlit.setUniform(vec3(0), "colour");

	program3DNormal.Use();
	program3DNormal.setUniform(camera.getViewMatrix() * projection, "vp");
	program3DNormal.setUniform(vec3(1, 1, 0), "colour");

	for (int i = 0; i < Editable::edibles.size(); i++)
	{
		glBindVertexArray(Editable::edibles[i]->vao);

		program3D.Use();
		program3D.setUniform(Editable::edibles[i]->globalModelMatrix, "model");

		if (Editable::edibles[i]->albedo != 0)
		{
			program3D.setUniform(69, "isSampled");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Editable::edibles[i]->albedo);
			program3D.setUniform(vec3(1, 1, 1), "colour");
		}
		else
		{
			program3D.setUniform(0, "isSampled");
			program3D.setUniform(vec3(0.5f,0.5f,0.5f), "colour");
		}

		glDrawElements(GL_TRIANGLES, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);

		if (showVertices==0||Editable::edibles[i] != selectedEditable)
			continue;

		//render lines and points
		program3DUnlit.Use();

		program3DUnlit.setUniform(Editable::edibles[i]->globalModelMatrix, "model");

		program3DUnlit.setUniform(vec3(0, 0, 1), "colour");
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		program3DUnlit.setUniform(vec3(1, 0.85f, 0), "colour");
		glDrawArrays(GL_POINTS, 0, Editable::edibles[i]->vertices.size());

		//render normals
		if (showNormals == 0)
			continue;

		program3DNormal.Use();
		program3DNormal.setUniform(Editable::edibles[i]->globalModelMatrix, "model");
		glDrawElements(GL_TRIANGLES, Editable::edibles[i]->indices.size(), GL_UNSIGNED_INT, NULL);
	}

	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	
	if (selectedEditable != NULL && showVertices!=0&&selectedVertexIDs.size() != 0)//render selected points
	{
		program3DUnlit.Use();

		program3DUnlit.setUniform(vec3(1, 0, 0), "colour");
		program3DUnlit.setUniform(selectedEditable->globalModelMatrix, "model");
		glBindVertexArray(selectedEditable->vao);

		for (int i = 0; i < selectedVertexIDs.size(); i++)
		{
			glDrawArrays(GL_POINTS, selectedVertexIDs[i], 1);
		}
	}
	if (selectedEditable != NULL && showVertices == 0)//render object highlight
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);

		glBindVertexArray(selectedEditable->vao);

		//write stencil buffer
		glStencilMask(0xFF);
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		program3D.Use();
		program3D.setUniform(selectedEditable->globalModelMatrix, "model");
		program3D.setUniform(selectedEditable->albedo != 0 ? 69 : 0, "isSampled");
		program3D.setUniform(selectedEditable->albedo != 0 ? vec3(1, 1, 1): vec3(0.5f, 0.5f, 0.5f), "colour");
		if (selectedEditable->albedo != 0){
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, selectedEditable->albedo);
		}

		glDrawElements(GL_TRIANGLES, selectedEditable->indices.size(), GL_UNSIGNED_INT, NULL);

		//draw highlight
		glStencilMask(0x00);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

		glDisable(GL_DEPTH_TEST);

		program3DUnlit.Use();
		program3DUnlit.setUniform(ScaleMatrix(vec3(1.02f, 1.02f, 1.02f))*selectedEditable->globalModelMatrix, "model");
		program3DUnlit.setUniform(vec3(0,1,1), "colour");

		glDrawElements(GL_TRIANGLES, selectedEditable->indices.size(), GL_UNSIGNED_INT, NULL);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glDisable(GL_STENCIL_TEST);
	}

	glBindVertexArray(0);
	glUseProgram(0);

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
}

void Editable::render2D(const Camera& cum, vec2 bottomLeft, vec2 topRight, float zoom)
{
	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);

	glPointSize(5);
	glLineWidth(2);

	float aspectXY = (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y);
	mat4 projection = OrthoMatrix(-0.5f*aspectXY*zoom, 0.5f*aspectXY*zoom, -0.5f*zoom, 0.5f*zoom, 0,10);
	
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


	//render object
	if (selectedEditable != NULL)
	{
		program2D.Use();
		program2D.setUniform(projection, "projection");
		program2D.setUniform(cum.getViewMatrix(), "view");
		
		glBindVertexArray(selectedEditable->vao);


		program2D.setUniform(vec3(0, 0, 1), "colour");
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, selectedEditable->indices.size(), GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		program2D.setUniform(vec3(1, 0.85f, 0), "colour");
		glDrawElements(GL_POINTS, selectedEditable->indices.size(), GL_UNSIGNED_INT, NULL);

		if (selectedVertexIDs.size() != 0)
		{
			program2D.setUniform(vec3(1, 0, 0), "colour");
			
			for(int i=0;i<selectedVertexIDs.size();i++)
				glDrawArrays(GL_POINTS, selectedVertexIDs[i], 1);
		}
	}

	glBindVertexArray(0);
	glUseProgram(0);
}


//texture
EditableTexture::EditableTexture(unsigned int texture, const char* path)
{
	this->texture = texture;
	if (strlen(path) > 199)
		strcpy(this->path, path + strlen(path) - 199);//stores the last 200 characters of the path, because that is what it identifies it
	else
		strcpy(this->path, path);
	this->referenceCount = 1;
}


unsigned int Editable::importTexture(const char* path)
{
	static char pathKey[200];

	if (strcmp(path, "") == 0)
		return 0;

	//trim input for the search
	if (strlen(path) > 199)
		strcpy(pathKey, path + strlen(path) - 199);
	else
		strcpy(pathKey, path);

	//check if the texture is already loaded
	for (int i = 0; i < Editable::textures.size(); i++)
	{
		if (strcmp(pathKey, Editable::textures[i].path) == 0)
		{
			Editable::textures[i].referenceCount++;
			return Editable::textures[i].texture;
		}
	}

	unsigned int newTex= TextureLoader::load(path, GL_LINEAR, 69);
	if (newTex == 0)
		return 0;

	Editable::textures.push_back(EditableTexture(newTex, path));
	return newTex;
}

void Editable::releaseTexture(unsigned int texture)
{
	int index = -1;

	for (int i = 0; i < Editable::textures.size(); i++)
	{
		if (Editable::textures[i].texture == texture)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
		return;


	Editable::textures[index].referenceCount--;

	if (Editable::textures[index].referenceCount == 0)
	{
		glDeleteTextures(1, &Editable::textures[index].texture);
		Editable::textures.erase(Editable::textures.begin() + index);
	}
}


void Editable::incrementTextureReference(unsigned int texture)
{
	if (texture == 0)
		return;

	for (int i = 0; i < Editable::textures.size(); i++)
	{
		if (texture == Editable::textures[i].texture)
		{
			Editable::textures[i].referenceCount++;
			return;
		}
	}
}
