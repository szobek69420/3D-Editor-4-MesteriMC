#ifndef EDITABLE_H
#define EDITABLE_H

#include <vector>
#include "../framework.h"

struct VertexData {
	vec3 position;
	vec2 uv;
};

class Editable {

public:
	enum Preset {
		CUBE, CYLINDER, SPHERE
	};

	//non static parts
private:
	char name[100];

	vec3 localPosition;
	vec3 localScale;
	vec3 localRotation;
	mat4 globalModelMatrix;

	Editable* parent;
	std::vector<Editable*> children;

	unsigned int vao, vbo, ebo;
	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;

private:
	Editable(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	Editable(Preset preset);
	~Editable();

	mat4 calculateLocalMatrix();

public:
	void recalculateGlobalMatrix(const mat4& parentGlobalModel); //recalculates also the children
	void setParent(Editable* parent);//NULL means no fatherless
	void setName(const char* name);

	//static part
private:
	static std::vector<Editable*> edibles;

public:
	static void initialize();
	static void deinitialize();

	static Editable* add(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	static Editable* add(Preset preset);
	static void remove(Editable* edible);
	static void removeWithChildren(Editable* edible);

	static void renderHierarchy();

private:
	static void renderHierarchyItem(Editable* edible);//helper function for renderHierarchy
};

#endif