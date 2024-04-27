#ifndef ROLLBACK_H
#define ROLLBACK_H

#include "../framework.h"
#include "../Quaternion/quaternion.h"

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


class RollbackOrientation : public RollbackItem{
public:
	unsigned int eid;
	vec3 localPosition;
	vec3 localScale;
	quat localRotation;

	RollbackOrientation(unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation);

	void rollback();
};



#endif