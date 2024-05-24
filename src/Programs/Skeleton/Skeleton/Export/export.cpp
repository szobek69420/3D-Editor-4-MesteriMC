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

	void collectTexturesHelper(std::vector<char*>& textures, SerializableTreeNode* node)
	{
		if (strlen(node->data.albedoPath) != 0)//no texture
		{
			for(int i=0;i<textures.size();i++)
				if (strcmp(node->data.albedoPath, textures[i]) == 0)
				{
					char* temp = (char*)malloc(strlen(node->data.albedoPath) + 1);
					strcpy(temp, node->data.albedoPath);
					textures.push_back(temp);
					break;
				}
		}

		for (int i = 0; i < node->children.size(); i++)
			collectTexturesHelper(textures, &(node->children[i]));
	}

	void printTreeToFileHelper(FILE* file, const std::vector<char*> textures, SerializableTreeNode* node, const char* parentName)
	{
		fprintf(file, "#object\n");
		fprintf(file, "o %s\n", node->data.name);
		fprintf(file, "parent %s\n", parentName);

		fprintf(file, "#local transform\n");
		fprintf(file, "position %.5f %.5f %.5f\n", node->data.localPosition.x, node->data.localPosition.y, node->data.localPosition.z);
		fprintf(file, "rotation %.5f %.5f %.5f %.5f\n", node->data.localRotation.s, node->data.localRotation.x, node->data.localRotation.y, node->data.localRotation.z);
		fprintf(file, "scale %.5f %.5f %.5f\n", node->data.localScale.x, node->data.localScale.y, node->data.localScale.z);
	
		fprintf(file, "#object textures\n");
		for(int i=0;i<textures.size();i++)//albedo
			if (strcmp(node->data.albedoPath, textures[i]) == 0)
			{
				fprintf(file, "texture_albedo %d\n", i);
				goto albedo_found;
			}

		fprintf(file, "texture_albedo -1\n");

albedo_found:

		fprintf(file, "#vertices\n");
		for (int i = 0; i < node->data.vertices.size(); i++)
		{
#define vertex node->data.vertices[i]
			fprintf(file, "vp %.5f %.5f %.5f\n", vertex.position.x, vertex.position.y, vertex.position.z);
			fprintf(file, "vuv %.5f %.5f\n", vertex.uv.x, vertex.uv.y);
#undef vertex
		}

		fprintf(file, "#faces\n");
		for (int i = 0; i < node->data.indices.size(); i+=3)
		{
			fprintf(file, "f %d %d %d\n", node->data.indices[i], node->data.indices[i + 1], node->data.indices[i + 2]);
		}

		//print children
		for (int i = 0; i < node->children.size(); i++)
			printTreeToFileHelper(file, textures, &(node->children[i]), node->data.name);
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

	void collectTexturesAlbedo(std::vector<char*>& textures)//dynamically allocates the char*s
	{
		for (int i = 0; i < objects.size(); i++)
			collectTexturesHelper(textures, &(objects[i]));
	}

	void printTreeToFile(FILE* file, const std::vector<char*> textures)
	{
		for (int i = 0; i < objects.size(); i++)
			printTreeToFileHelper(file, textures, &(objects[i]), "FATHERLESS");
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
		tree.addToTree(parent);
#undef parent
	}

	//collect textures
	std::vector<char*> albedos;
	tree.collectTexturesAlbedo(albedos);

	//print to file
	FILE* file = fopen(filePath, "w");
	if (file == NULL)
	{
		printf("The file %s could not be created\n", filePath);
		goto end;
	}

	//print textures
	fprintf(file, "#textures\n");
	for (int i = 0; i < albedos.size(); i++)
		fprintf(file, "texture %d %s\n", i, albedos[i]);

	//print animations...

	//print objects
	tree.printTreeToFile(file, albedos);

	fclose(file);

end:
	for (int i = 0; i < albedos.size(); i++)
		free(albedos[i]);
}