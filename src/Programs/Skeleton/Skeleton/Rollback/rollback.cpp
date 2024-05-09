#include "rollback.h"

#include <vector>
#include <string.h>
#include <stdlib.h>

#include "../framework.h"
#include "../Quaternion/quaternion.h"

#include "../Editable/editable.h"

extern std::vector<Editable*> editablesInScene;

static unsigned int bufferInitialized = 0;
static unsigned int currentIndexInRollbackBuffer = 0;

RollbackItem::RollbackItem(const char* _opName, unsigned int _eid){
	strcpy_s(this->opName, ROLLBACK_MAX_NAME_LENGTH, _opName);
	this->eid = _eid;
}

RollbackItem* RollbackItem::rollbackBuffer[ROLLBACK_MAX_COUNT];

const char* RollbackItem::getName() {
	return this->opName;
}

void RollbackItem::addToBuffer(const RollbackItem& hmm)
{
	if (bufferInitialized == 0)
	{
		memset(RollbackItem::rollbackBuffer, 0, sizeof(RollbackItem::rollbackBuffer));
		bufferInitialized = 69;
	}

	RollbackItem* ri=NULL;

	if (dynamic_cast<const RollbackOrientationObject*>(&hmm) != NULL)
	{
		ri = (RollbackOrientationObject*)malloc(sizeof(RollbackOrientationObject));
		if (ri == NULL)
			return;

		memcpy(ri, &hmm, sizeof(RollbackOrientationObject));
	}
	else if (dynamic_cast<const RollbackOrientationVertex*>(&hmm) != NULL)
	{
		const RollbackOrientationVertex* temp = dynamic_cast<const RollbackOrientationVertex*>(&hmm);
		ri = new RollbackOrientationVertex(temp->opName, temp->eid, temp->vertices, temp->indices);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackDeleteObject*>(&hmm) != NULL)
	{
		const RollbackDeleteObject* temp = dynamic_cast<const RollbackDeleteObject*>(&hmm);
		ri = new RollbackDeleteObject("", NULL);
		if (ri == NULL)
			return;

		strcpy(((RollbackDeleteObject*)ri)->opName, temp->opName);
		((RollbackDeleteObject*)ri)->edible = temp->edible;
	}
	else if (dynamic_cast<const RollbackAddObject*>(&hmm) != NULL)
	{
		ri = (RollbackAddObject*)malloc(sizeof(RollbackAddObject));
		if (ri == NULL)
			return;

		memcpy(ri, &hmm, sizeof(RollbackAddObject));
	}

	if (ri == NULL)
		return;

	currentIndexInRollbackBuffer++;
	if (currentIndexInRollbackBuffer >= ROLLBACK_MAX_COUNT)
		currentIndexInRollbackBuffer = 0;

	if (currentIndexInRollbackBuffer != NULL)
		delete RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer];

	RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer] = ri;
}

void RollbackItem::undo()
{
	if (RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer] == NULL)
		return;

	RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer]->rollback();
	delete RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer];
	RollbackItem::rollbackBuffer[currentIndexInRollbackBuffer] = NULL;

	if (currentIndexInRollbackBuffer == 0)
		currentIndexInRollbackBuffer = ROLLBACK_MAX_COUNT - 1;
	else
		currentIndexInRollbackBuffer--;
}


RollbackOrientationObject::RollbackOrientationObject(const char* _opName, unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation) :
	RollbackItem(_opName, _eid),
	localPosition(_localPosition), localScale(_localScale), localRotation(_localRotation)
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


RollbackOrientationVertex::RollbackOrientationVertex(const char* _opName, unsigned int _eid, const std::vector<VertexData>& _vertices, const std::vector<unsigned int>& _indices)
	: RollbackItem(_opName, _eid)
{
	this->vertices=_vertices;
	this->indices=_indices;
}

void RollbackOrientationVertex::rollback()
{
	for (unsigned int i = 0; i < editablesInScene.size(); i++)
	{
#define e editablesInScene[i]
		if (e->getId() != this->eid)
			continue;

		e->setVertexData(this->vertices, this->indices);
		break;
#undef e
	}
}

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

void RollbackDeleteObject::rollback()
{
	Editable* realEdible=Editable::add(&edible);
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

RollbackAddObject::RollbackAddObject(const char* _opName, unsigned int _eid) :RollbackItem(_opName, _eid) {}

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

//cleanup
class RollbackCleanup {
public:
	RollbackCleanup(){}
	~RollbackCleanup() {
		for (int i = 0; i < ROLLBACK_MAX_COUNT; i++)
		{
			if (RollbackItem::rollbackBuffer[i] != NULL)
			{
				delete RollbackItem::rollbackBuffer[i];
				RollbackItem::rollbackBuffer[i] = NULL;
			}
		}
	}
};

static RollbackCleanup cleanup;