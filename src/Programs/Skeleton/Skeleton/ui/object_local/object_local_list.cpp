#include "object_local_list.h"

#include <vector>

#include "../../framework.h"
#include "../../Editable/editable.h"
#include "../../System/system.h"

#include "../../ImGui/imgui.h"

#include "../../Rollback/rollback.h"

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
	Editable* edible = NULL;
	switch (currentChildList)
	{
	case ChildList::CREATE_OBJECT:
		ImGui::SetNextWindowPos(ImVec2(currentPos.x + ObjectLocalList::width, currentPos.y));
		ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, 80));
		ImGui::Begin("object_local_list_child", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::Button("Cuba")) {
			edible = Editable::add(Editable::Preset::CUBE);
		}
		if (ImGui::Button("Ball")) {
			edible = Editable::add(Editable::Preset::SPHERE);
		}
		if (ImGui::Button("Cylinder")) {
			edible = Editable::add(Editable::Preset::CYLINDER);
		}

		if (edible != NULL)
		{
			editablesInScene.push_back(edible);
			RollbackItem::addToBuffer(RollbackAddObject("create object", edible->getId()));
		}
		ImGui::End();
		break;

	case ChildList::CREATE_OBJECT_CHILD:
		ImGui::SetNextWindowPos(ImVec2(currentPos.x + ObjectLocalList::width, currentPos.y+22));
		ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, 80));
		ImGui::Begin("object_local_list_child", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
		
		Editable* edible = NULL;
		RollbackComposite commands = RollbackComposite("create object");

		if (ImGui::Button("Cuba")) {
			edible = Editable::add(Editable::Preset::CUBE);
		}
		if (ImGui::Button("Ball")) {
			edible = Editable::add(Editable::Preset::SPHERE);
		}
		if (ImGui::Button("Cylinder")) {
			edible = Editable::add(Editable::Preset::CYLINDER);
		}

		if (edible != NULL)
		{
			edible->setPosition(-1 * selectedEditable->getPosition());
			editablesInScene.push_back(edible);
			edible->setParent(selectedEditable);
			edible->recalculateGlobalMatrix();

			RollbackAddObject command1 = RollbackAddObject("add object", edible->getId());
			RollbackParentChange command2 = RollbackParentChange("change parent", edible->getId(), selectedEditable->getId());//itt erre nem feltetlenul van szukseg
			commands.addItem(&command1);
			commands.addItem(&command2);
			RollbackItem::addToBuffer(commands);
		}

		ImGui::End();
		break;
	}

	ImGui::PopStyleColor(3);
}


int ObjectLocalList::isOpen()
{
	return isVisible;
}