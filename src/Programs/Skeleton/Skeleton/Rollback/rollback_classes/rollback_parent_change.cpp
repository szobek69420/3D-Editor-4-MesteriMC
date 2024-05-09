#include "../rollback.h"
#include "../../Editable/editable.h"

extern std::vector<Editable*> editablesInScene;

RollbackParentChange::RollbackParentChange(const char* _opName, unsigned int _eid, unsigned int _parentIdOld) : RollbackItem(_opName, _eid), parentId(_parentIdOld)
{}

RollbackParentChange::RollbackParentChange(const RollbackParentChange& other) : RollbackItem(other), parentId(other.parentId)
{}

void RollbackParentChange::rollback()
{
	Editable* edible = NULL, * parent = NULL;

	for (int i = 0; i < editablesInScene.size(); i++)
	{
		if (this->eid == editablesInScene[i]->getId())
			edible = editablesInScene[i];
		if (this->parentId == editablesInScene[i]->getId())
			parent = editablesInScene[i];
	}

	if (edible == NULL)
		return;

	if (edible->getParent() != NULL)
		edible->getParent()->removeChild(edible);

	if (parent != NULL)
		parent->addChild(edible);
	edible->setParent(parent);
}