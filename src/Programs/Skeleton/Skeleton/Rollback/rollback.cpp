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

static unsigned int currentCommandId = 1;

RollbackItem* RollbackItem::rollbackBuffer[ROLLBACK_MAX_COUNT];


RollbackItem::RollbackItem(const char* _opName, unsigned int _eid):commandId(currentCommandId++)
{
	strcpy_s(this->opName, ROLLBACK_MAX_NAME_LENGTH, _opName);
	this->eid = _eid;
}

RollbackItem::RollbackItem(const RollbackItem& other) :commandId(other.commandId)
{
	strcpy(this->opName, other.opName);
	this->eid = other.eid;
}

bool RollbackItem::operator==(const RollbackItem& other) const
{
	if (this->commandId == other.commandId)
		return true;
	return false;
}


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


	if (dynamic_cast<const RollbackComposite*>(&hmm) != NULL)
	{
		const RollbackComposite* temp = dynamic_cast<const RollbackComposite*>(&hmm);
		ri = new RollbackComposite(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackOrientationObject*>(&hmm) != NULL)
	{
		const RollbackOrientationObject* temp = dynamic_cast<const RollbackOrientationObject*>(&hmm);
		ri = new RollbackOrientationObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackOrientationVertex*>(&hmm) != NULL)
	{
		const RollbackOrientationVertex* temp = dynamic_cast<const RollbackOrientationVertex*>(&hmm);
		ri = new RollbackOrientationVertex(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackDeleteObject*>(&hmm) != NULL)
	{
		const RollbackDeleteObject* temp = dynamic_cast<const RollbackDeleteObject*>(&hmm);
		ri = new RollbackDeleteObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackAddObject*>(&hmm) != NULL)
	{
		const RollbackAddObject* temp = dynamic_cast<const RollbackAddObject*>(&hmm);
		ri = new RollbackAddObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackParentChange*>(&hmm) != NULL)
	{
		const RollbackParentChange* temp = dynamic_cast<const RollbackParentChange*>(&hmm);
		ri = new RollbackParentChange(*temp);
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