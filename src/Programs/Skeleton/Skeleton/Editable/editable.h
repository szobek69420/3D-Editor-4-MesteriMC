#ifndef EDITABLE_H
#define EDITABLE_H

#include <vector>
#include "../framework.h"
#include "../Camera/camera.h"

class VertexData {
public:
	vec3 position;
	vec2 uv;

	VertexData(vec3 _pos, vec2 _uv) :position(_pos), uv(_uv) {}
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

	unsigned int albedo=0;

private:
	Editable(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	~Editable();

	mat4 calculateLocalMatrix();

public:
	void recalculateGlobalMatrix(const mat4& parentGlobalModel); //recalculates also the children
	void setParent(Editable* parent);//NULL means no fatherless
	void setName(const char* name);

	const std::vector<VertexData>& getVertices();
	void setVertexData(unsigned int vertexID, VertexData data);

	mat4 getGlobalMatrix();

	void setAlbedo(unsigned int texture);
	unsigned int getAlbedo();

	//static part
private:
	static std::vector<Editable*> edibles;

public:
	static void initialize();
	static void deinitialize();

	static Editable* add(VertexData* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	static Editable* add(Editable::Preset preset);
	static void remove(Editable* edible);
	static void removeWithChildren(Editable* edible);

	static void renderHierarchy();
	static void render3D(const Camera& camera, vec2 bottomLeft, vec2 topRight);
	static void render2D(const Camera& cum, vec2 bottomLeft, vec2 topRight);

private:
	static void renderHierarchyItem(Editable* edible);//helper function for renderHierarchy
};

#endif