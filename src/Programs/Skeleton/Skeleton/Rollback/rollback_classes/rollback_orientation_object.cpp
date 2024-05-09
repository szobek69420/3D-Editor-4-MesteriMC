#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>


extern std::vector<Editable*> editablesInScene;

RollbackOrientationObject::RollbackOrientationObject(const char* _opName, unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation) :
	RollbackItem(_opName, _eid),
	localPosition(_localPosition), localScale(_localScale), localRotation(_localRotation)
{}

RollbackOrientationObject::RollbackOrientationObject(const RollbackOrientationObject& other) :
	RollbackItem(other),
	localPosition(other.localPosition), localScale(other.localScale), localRotation(other.localRotation)
{}

void RollbackOrientationObject::rollback()
{
	for (unsigned int i = 0; i < editablesInScene.size(); i++)
	{
#define e editablesInScene[i]
		if (e->getId() != this->eid)
			continue;

		e->setPosition(this->localPosition);
		e->setRotation(this->localRotation);
		e->setScale(this->localScale);
		e->recalculateGlobalMatrix();
		break;
#undef e
	}
}