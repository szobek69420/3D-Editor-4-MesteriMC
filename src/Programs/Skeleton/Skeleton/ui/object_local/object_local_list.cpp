#include "object_local_list.h"

#include <vector>

#include "../../framework.h"
#include "../../Editable/editable.h"
#include "../../System/system.h"

#include "../../ImGui/imgui.h"

extern std::vector<Editable*> editablesInScene;
extern Editable* selectedEditable;
extern std::vector<unsigned int> selectedVertexIDs;

static enum ChildList{
	NONE, CREATE_OBJECT,CREATE_OBJECT_CHILD
};

static int isVisible = 0;
static int currentChildList = ChildList::NONE;
static ImVec2 currentPos;

void ObjectLocalList::open(vec2 mousePosInScreenSpace)
{
	currentPos = ImVec2(mousePosInScreenSpace.x, mousePosInScreenSpace.y);
	isVisible = 69;
}

void ObjectLocalList::close()
{
	currentChildList = ChildList::NONE;
	isVisible = 0;
}

void ObjectLocalList::render(vec2 bottomLeft, vec2 topRight)
{
	if (isVisible == 0)
		return;

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));



	ImGui::SetNextWindowPos(currentPos);
	ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, ObjectLocalList::height));
	ImGui::Begin("object_local_list", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
	

	if (ImGui::Button("Create"))
		currentChildList = ChildList::CREATE_OBJECT;

	if (selectedEditable == NULL)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

	if (ImGui::Button("Create as child")&&selectedEditable!=NULL)
		currentChildList = ChildList::CREATE_OBJECT_CHILD;
	if (ImGui::Button("Delete") && selectedEditable != NULL)
	{
		int index = -1;
		for (int i = 0; i < editablesInScene.size(); i++)
		{
			if (editablesInScene[i] == selectedEditable)
			{
				index = i;
				break;
			}
		}

		if (index != -1)
		{
			editablesInScene.erase(editablesInScene.begin() + index);
			Editable::removeWithChildren(selectedEditable);
			selectedEditable = NULL;
			ObjectLocalList::close();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));//hogy ne legyen tul sokszor meghivva a popstylecolor
		}
	}

	if (selectedEditable == NULL)
		ImGui::PopStyleColor();

	ImGui::End();

	//child lists
	switch (currentChildList)
	{
	case ChildList::CREATE_OBJECT:
		ImGui::SetNextWindowPos(ImVec2(currentPos.x + ObjectLocalList::width, currentPos.y));
		ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, 80));
		ImGui::Begin("object_local_list_child", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::Button("Cuba"))
		{
			Editable* edible = Editable::add(Editable::Preset::CUBE);
			editablesInScene.push_back(edible);
		}
		if (ImGui::Button("Ball"));
		if (ImGui::Button("Cylinder"));
		ImGui::End();
		break;

	case ChildList::CREATE_OBJECT_CHILD:
		ImGui::SetNextWindowPos(ImVec2(currentPos.x + ObjectLocalList::width, currentPos.y+22));
		ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, 80));
		ImGui::Begin("object_local_list_child", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::Button("Cuba"));
		if (ImGui::Button("Ball"));
		if (ImGui::Button("Cylinder"));
		ImGui::End();
		break;
	}

	ImGui::PopStyleColor(3);
}


int ObjectLocalList::isOpen()
{
	return isVisible;
}