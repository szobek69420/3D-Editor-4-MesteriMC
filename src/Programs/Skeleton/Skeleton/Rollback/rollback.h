#ifndef ROLLBACK_H
#define ROLLBACK_H

#include "../framework.h"
#include "../Quaternion/quaternion.h"
#include "../Editable/editable.h"

#include <vector>

#define ROLLBACK_MAX_COUNT 50
#define ROLLBACK_MAX_NAME_LENGTH 20

class RollbackCleanup;

class RollbackItem {

	friend RollbackCleanup;

public:
	unsigned int eid;//edible id
	char opName[20];

	RollbackItem(const char* _opName, unsigned int _eid);
	RollbackItem(const RollbackItem& other);

public:
	virtual void rollback() = 0;
	const char* getName();

private:
	static RollbackItem* rollbackBuffer[ROLLBACK_MAX_COUNT];

public:
	static void addToBuffer(const RollbackItem& hmm);
	static void undo();
};


class RollbackOrientationObject : public RollbackItem{
public:
	vec3 localPosition;
	vec3 localScale;
	quat localRotation;

	RollbackOrientationObject(const char* _opName, unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation);
	RollbackOrientationObject(const RollbackOrientationObject& other);

	void rollback();
};

class RollbackOrientationVertex : public RollbackItem {
public:
	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;

	RollbackOrientationVertex(const char* _opName, unsigned int _eid, const std::vector<VertexData>& _vertices, const std::vector<unsigned int>& indices);
	RollbackOrientationVertex(const RollbackOrientationVertex& other);

	void rollback();
};

class RollbackDeleteObject : public RollbackItem {
public:
	SerializableEditable edible;

	RollbackDeleteObject(const char* _opName, const Editable* _edible);
	RollbackDeleteObject(const RollbackDeleteObject& other);

	void rollback();
};

class RollbackAddObject : public RollbackItem {
public:
	RollbackAddObject(const char* _opName, unsigned int _eid);
	RollbackAddObject(const RollbackAddObject& other);
	void rollback();
};

class RollbackParentChange : public RollbackItem {
public:
	unsigned int parentId;

	RollbackParentChange(const char* _opName, unsigned int _eid, unsigned int _parentIdOld);
	void rollback();
};



#endif