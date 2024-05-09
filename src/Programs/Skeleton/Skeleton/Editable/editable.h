#ifndef EDITABLE_H
#define EDITABLE_H

#include <vector>
#include "../framework.h"
#include "../Camera/camera.h"
#include "../Quaternion/quaternion.h"

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

class SerializableEditable;

class Editable {

	friend SerializableEditable;


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
	Quaternion localRotation;
	mat4 globalModelMatrix;
	mat4 inverseGlobalModelMatrix;

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
	mat4 calculateInverseLocalMatrix();

	void refreshVertexBuffer();
	void refreshIndexBuffer();

public:
	void recalculateGlobalMatrix(); //recalculates also the children and the inverse matrix
	
	const char* getName();
	void setName(const char* name);

	Editable* getParent();
	void setParent(Editable* parent);//NULL means fatherless

	const std::vector<Editable*> getChildren();
	void addChild(Editable* child);
	void removeChild(const Editable* child);//doesn't set the parent of the child

	const std::vector<VertexData>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;

	void setVertexData(unsigned int vertexID, VertexData data);
	void setVertexData(const std::vector<VertexData>& _newVertices, const std::vector<unsigned int>& _newIndices);

	mat4 getGlobalMatrix();
	mat4 getInverseGlobalMatrix();

	void setAlbedo(const char* albedoPath = "");
	unsigned int getAlbedo();
	const char* getAlbedoPath();

	vec3 getPosition();
	void setPosition(const vec3& position);

	Quaternion getRotation();
	void setRotation(const Quaternion& rotation);

	vec3 getScale();
	void setScale(const vec3& scale);

	unsigned int getId() const { return id; }
	void setId(unsigned int id) { this->id = id; }

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
	static Editable* add(const SerializableEditable* se);//the parent-child connections will not be reconstructed here
	static void remove(Editable* edible);
	static void removeWithChildren(Editable* edible);

	static Editable* clone(Editable* edible);

	static std::vector<Editable*> getEditables();

	static void renderHierarchy();
	static void render3D(const Camera& camera, vec2 bottomLeft, vec2 topRight, int showVertices, int showNormals);
	static void render2D(const Camera& cum, vec2 bottomLeft, vec2 topRight, float zoom);

	static unsigned int importTexture(const char* path);
	static void releaseTexture(unsigned int texture);

private:
	static void printEditableToFile(const SerializableEditable* edible, FILE* file);//helper function for "saveAs"
	static SerializableEditable readEditableFromFile(FILE* file);
	static Editable* serializableToEditable(const SerializableEditable* se);//the parents and children are ignored here
	static void renderHierarchyItem(Editable* edible);//helper function for renderHierarchy
	static void incrementTextureReference(unsigned int texture); //helper function for clone
};


class SerializableEditable {
public:
	unsigned int id;
	char name[EDIBLE_NAME_MAX_LENGTH];
	vec3 localPosition;
	vec3 localScale;
	quat localRotation;

	unsigned int parentId;//the id of the parent (0 is fatherless)
	std::vector<unsigned int> childId;//the ids of the children

	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;

	char albedoPath[EDIBLE_PATH_MAX_LENGTH];

	SerializableEditable();
	SerializableEditable(const Editable* edible);
};

#endif