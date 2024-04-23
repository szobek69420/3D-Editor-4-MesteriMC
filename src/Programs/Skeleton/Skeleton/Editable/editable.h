#ifndef EDITABLE_H
#define EDITABLE_H

#include <vector>
#include "../framework.h"
#include "../Camera/camera.h"

#define EDIBLE_NAME_MAX_LENGTH 100
#define EDIBLE_PATH_MAX_LENGTH 200

class VertexData {
public:
	vec3 position;
	vec2 uv;

	VertexData() : position(vec3()), uv(vec2()) {}
	VertexData(vec3 _pos, vec2 _uv) :position(_pos), uv(_uv) {}
};

class EditableTexture {
public:
	unsigned int texture;
	char path[EDIBLE_PATH_MAX_LENGTH];
	int referenceCount;

	EditableTexture(unsigned int texture, const char* path);
};

class Editable {

public:
	enum Preset {
		CUBE, CYLINDER, SPHERE
	};

	//non static parts
private:
	unsigned int id;
	char name[EDIBLE_NAME_MAX_LENGTH];

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
	char albedoPath[EDIBLE_PATH_MAX_LENGTH];

private:
	Editable(const VertexData* vertices, const unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	~Editable();

	mat4 calculateLocalMatrix();

	void refreshVertexBuffer();
	void refreshIndexBuffer();

public:
	void recalculateGlobalMatrix(); //recalculates also the children
	void setParent(Editable* parent);//NULL means no fatherless
	void setName(const char* name);

	const std::vector<VertexData>& getVertices() const;
	void setVertexData(unsigned int vertexID, VertexData data);

	const std::vector<unsigned int>& getIndices() const;

	mat4 getGlobalMatrix();

	void setAlbedo(const char* albedoPath = "");
	unsigned int getAlbedo();
	const char* getAlbedoPath();

	vec3 getPosition();
	void setPosition(const vec3& position);

	vec3 getRotation();
	void setRotation(const vec3& rotation);

	vec3 getScale();
	void setScale(const vec3& scale);

	void addVertex(vec3 pos, vec2 uv = vec2(0, 0));
	void removeVertex(unsigned int vertexID);
	void addFace(unsigned int vertexID0, unsigned int vertexID1, unsigned int vertexID2);

	//static part
private:
	static std::vector<Editable*> edibles;
	static std::vector<EditableTexture> textures;

public:
	static void initialize();
	static void deinitialize();

	static void saveAs(const char* filePath);
	static void importFrom(const char* filePath);

	static Editable* add(const VertexData* vertices, const unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	static Editable* add(Editable::Preset preset);
	static void remove(Editable* edible);
	static void removeWithChildren(Editable* edible);

	static Editable* clone(Editable* edible);

	static void renderHierarchy();
	static void render3D(const Camera& camera, vec2 bottomLeft, vec2 topRight, int showVertices);
	static void render2D(const Camera& cum, vec2 bottomLeft, vec2 topRight, float zoom);

	static unsigned int importTexture(const char* path);
	static void releaseTexture(unsigned int texture);

private:
	static void printEditableToFile(Editable* edible, FILE* file);//helper function for "saveAs"
	static void renderHierarchyItem(Editable* edible);//helper function for renderHierarchy
	static void incrementTextureReference(unsigned int texture); //helper function for clone
};

#endif