#ifndef ROLLBACK_H
#define ROLLBACK_H

#include "../framework.h"
#include "../Quaternion/quaternion.h"
#include "../Editable/editable.h"

#include <vector>

#define ROLLBACK_MAX_COUNT 50

class RollbackCleanup;

class RollbackItem {

	friend RollbackCleanup;

protected:
	RollbackItem();

public:
	virtual void rollback() = 0;

private:
	static RollbackItem* rollbackBuffer[ROLLBACK_MAX_COUNT];

public:
	static void addToBuffer(const RollbackItem& hmm);
	static void undo();
};


class RollbackOrientationObject : public RollbackItem{
public:
	unsigned int eid;//edible id
	vec3 localPosition;
	vec3 localScale;
	quat localRotation;

	RollbackOrientationObject(unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation);

	void rollback();
};

class RollbackOrientationVertex : public RollbackItem {
public:
	unsigned int eid;//edible id
	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;

	RollbackOrientationVertex(unsigned int _edi, const std::vector<VertexData>& _vertices, const std::vector<unsigned int>& indices);

	void rollback();
};



#endif