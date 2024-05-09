#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>

RollbackComposite::RollbackComposite(const char* _opName) :RollbackItem(_opName, 0) {}

RollbackComposite::RollbackComposite(const RollbackComposite& other) :RollbackItem(other)
{
	this->commands = other.commands;
}

void RollbackComposite::addItem(RollbackItem& item)
{
	this->commands.push_back(item);
}

void RollbackComposite::removeItem(RollbackItem& item)
{
	for (int i = 0; i < this->commands.size(); i++)
	{
		if (this->commands[i] == item)
		{
			this->commands.erase(this->commands.begin() + i);
			break;
		}
	}
}

void RollbackComposite::rollback()
{
	for (int i = this->commands.size() - 1; i >= 0; i--)
		this->commands[i].rollback();
}