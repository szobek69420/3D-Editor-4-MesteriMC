#include "../rollback.h"
#include "../../Editable/editable.h"

#include <vector>


extern std::vector<Editable*> editablesInScene;

RollbackOrientationVertex::RollbackOrientationVertex(const char* _opName, unsigned int _eid, const std::vector<VertexData>& _vertices, const std::vector<unsigned int>& _indices)
	: RollbackItem(_opName, _eid)
{
	this->vertices = _vertices;
	this->indices = _indices;
}

RollbackOrientationVertex::RollbackOrientationVertex(const RollbackOrientationVertex& other) :
	RollbackItem(other), vertices(other.vertices), indices(other.indices)
{}

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