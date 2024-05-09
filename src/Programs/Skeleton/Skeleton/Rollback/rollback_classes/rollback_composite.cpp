#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>


RollbackComposite::RollbackComposite(const char* _opName) :RollbackItem(_opName, 0) {
	memset(this->commands, 0, sizeof(this->commands));
}

RollbackComposite::RollbackComposite(const RollbackComposite& other) :RollbackItem(other)
{
	memset(this->commands, 0, sizeof(this->commands));

	for (int i = 0; i < MAX_COMMAND_COUNT; i++)
	{
		if (other.commands[i] == NULL)
			continue;

		this->addItem(other.commands[i]);
	}
}

RollbackComposite::~RollbackComposite()
{
	for (int i = 0; i < MAX_COMMAND_COUNT; i++)
		delete commands[i];
}

void RollbackComposite::addItem(const RollbackItem* item)
{
	unsigned int index = MAX_COMMAND_COUNT;
	for (int i = 0; i < MAX_COMMAND_COUNT; i++)
	{
		if (this->commands[i] == NULL)
		{
			index = i;
			break;
		}
	}
	if (index == MAX_COMMAND_COUNT)
		return;


	RollbackItem* ri = NULL;


	if (dynamic_cast<const RollbackComposite*>(item) != NULL)
	{
		const RollbackComposite* temp = dynamic_cast<const RollbackComposite*>(item);
		ri = new RollbackComposite(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackOrientationObject*>(item) != NULL)
	{
		const RollbackOrientationObject* temp = dynamic_cast<const RollbackOrientationObject*>(item);
		ri = new RollbackOrientationObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackOrientationVertex*>(item) != NULL)
	{
		const RollbackOrientationVertex* temp = dynamic_cast<const RollbackOrientationVertex*>(item);
		ri = new RollbackOrientationVertex(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackDeleteObject*>(item) != NULL)
	{
		const RollbackDeleteObject* temp = dynamic_cast<const RollbackDeleteObject*>(item);
		ri = new RollbackDeleteObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackAddObject*>(item) != NULL)
	{
		const RollbackAddObject* temp = dynamic_cast<const RollbackAddObject*>(item);
		ri = new RollbackAddObject(*temp);
		if (ri == NULL)
			return;
	}
	else if (dynamic_cast<const RollbackParentChange*>(item) != NULL)
	{
		const RollbackParentChange* temp = dynamic_cast<const RollbackParentChange*>(item);
		ri = new RollbackParentChange(*temp);
		if (ri == NULL)
			return;
	}

	this->commands[index] = ri;
}

void RollbackComposite::removeItem(const RollbackItem* item)
{
	for (int i = 0; i < MAX_COMMAND_COUNT; i++)
	{
		if (*(this->commands[i]) == *item)
		{
			delete this->commands[i];
			this->commands[i] = NULL;
			break;
		}
	}
}

void RollbackComposite::rollback()
{
	for (int i = MAX_COMMAND_COUNT - 1; i >= 0; i--)
	{
		if (this->commands[i] == NULL)
			continue;

		//printf("%s\n", this->commands[i]->getName());
		this->commands[i]->rollback();
	}
}