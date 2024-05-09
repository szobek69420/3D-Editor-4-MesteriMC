#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>

extern std::vector<Editable*> editablesInScene;


RollbackDeleteObject::RollbackDeleteObject(const char* _opName, const Editable* _edible) :RollbackItem(_opName, 0)
{
	if (_edible != NULL)
	{
		eid = _edible->getId();
		edible = SerializableEditable(_edible);
	}
	else
	{
		eid = 0;
	}
}

RollbackDeleteObject::RollbackDeleteObject(const RollbackDeleteObject& other) :
	RollbackItem(other), edible(other.edible)
{}

void RollbackDeleteObject::rollback()
{
	Editable* realEdible = Editable::add(&edible);
	editablesInScene.push_back(realEdible);

	//reconstruct parent-child connections
	for (int i = 0; i < editablesInScene.size(); i++)
	{
		if (editablesInScene[i]->getId() == edible.parentId)
			realEdible->setParent(editablesInScene[i]);

		for (int j = 0; j < edible.childId.size(); j++)
		{
			if (editablesInScene[i]->getId() == edible.childId[j])
				realEdible->addChild(editablesInScene[i]);
		}
	}
	realEdible->recalculateGlobalMatrix();
}