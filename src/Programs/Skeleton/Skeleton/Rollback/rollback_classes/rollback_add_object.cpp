#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>

extern std::vector<Editable*> editablesInScene;


RollbackAddObject::RollbackAddObject(const char* _opName, unsigned int _eid) :RollbackItem(_opName, _eid) {}

RollbackAddObject::RollbackAddObject(const RollbackAddObject& other) :RollbackItem(other) {}

void RollbackAddObject::rollback()
{
	Editable* edible = NULL;

	for (int i = 0; i < editablesInScene.size(); i++)
	{
		if (eid == editablesInScene[i]->getId())
		{
			edible = editablesInScene[i];
			editablesInScene.erase(editablesInScene.begin() + i);
			break;
		}
	}

	if (edible == NULL)
		return;

	Editable::remove(edible);
}