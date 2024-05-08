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

RollbackItem::RollbackItem(){}

RollbackItem* RollbackItem::rollbackBuffer[ROLLBACK_MAX_COUNT];

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
		ri = new RollbackOrientationVertex(temp->eid, temp->vertices, temp->indices);
		if (ri == NULL)
			return;
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


RollbackOrientationObject::RollbackOrientationObject(unsigned int _eid, vec3 _localPosition, vec3 _localScale, quat _localRotation) :
	RollbackItem(),
	eid(_eid), localPosition(_localPosition), localScale(_localScale), localRotation(_localRotation)
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


RollbackOrientationVertex::RollbackOrientationVertex(unsigned int _id, const std::vector<VertexData>& _vertices, const std::vector<unsigned int>& _indices)
{
	this->eid = _id;
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