#include "../Export/export.h"
#include "../Editable/editable.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

struct SerializableTree {
public:
	struct SerializableTreeNode {
		SerializableEditable data;
		std::vector<SerializableTreeNode> children;
	};
	
	std::vector<SerializableTreeNode> objects;

private:
	void addToTreeHelper(const Editable* edible, SerializableTreeNode* parent)//adds children recursively
	{
		SerializableTreeNode stn;
		stn.data = SerializableEditable(edible);
		parent->children.push_back(stn);
		parent = &(parent->children[parent->children.size() - 1]);

		const std::vector<Editable*> children = edible->getChildren();
		for(int i=0;i<children.size();i++)
			addToTreeHelper(children[i], parent);
	}

public:
	void addToTree(const Editable* edible)//adds object to the tree including its children
	{
		SerializableTreeNode stn;
		stn.data = SerializableEditable(edible);
		objects.push_back(stn);

		SerializableTreeNode* parent = &(objects[objects.size() - 1]);
		const std::vector<Editable*> children = edible->getChildren();
		for (int i = 0; i < children.size(); i++)
			addToTreeHelper(children[i], parent);
	}
};

void Exporter::exportEditable(const char* filePath, void* parentsVoid, unsigned int parentCount)
{
	Editable** parents = (Editable**)parentsVoid;
	
	//construct serializable tree
	SerializableTree tree;

	SerializableTree::SerializableTreeNode* currentNode = NULL;
	for (int i = 0; i < parentCount; i++)
	{
#define parent parents[i]
		SerializableEditable serialized(parent);
#undef parent
	}

	//print to file
	FILE* file = fopen(filePath, "w");
	if (file == NULL)
	{
		printf("The file %s could not be created\n", filePath);
		return;
	}


	for (int i = 0; i < parentCount; i++)
	{
#define parent parents[i]
		SerializableEditable serialized(parent);
#undef parent
	}

	fclose(file);
}