#include "object_local_list.h"

#include <vector>

#include "../../framework.h"
#include "../../Editable/editable.h"
#include "../../System/system.h"

#include "../../ImGui/imgui.h"

extern std::vector<Editable*> editablesInScene;
extern Editable* selectedEditable;
extern std::vector<unsigned int> selectedVertexIDs;

static int isVisible = 0;
static ImVec2 currentPos;

void ObjectLocalList::open(vec2 mousePosInScreenSpace)
{
	currentPos = ImVec2(mousePosInScreenSpace.x, mousePosInScreenSpace.y);
	isVisible = 69;
}

void ObjectLocalList::close()
{
	isVisible = 0;
}

void ObjectLocalList::render(vec2 bottomLeft, vec2 topRight)
{
	if (isVisible == 0||topRight.x-bottomLeft.x<ObjectLocalList::width|| bottomLeft.y-topRight.y < ObjectLocalList::height)
		return;

	ImGui::SetNextWindowPos(currentPos);
	ImGui::SetNextWindowSize(ImVec2(ObjectLocalList::width, ObjectLocalList::height));
	ImGui::Begin("object_local_list", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
	
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

	if (ImGui::Button("Create"))
	{

	}

	if (selectedEditable == NULL)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
	if (ImGui::Button("Create as child")&&selectedEditable!=NULL)
	{

	}
	if (ImGui::Button("Delete") && selectedEditable != NULL)
	{

	}
	if (selectedEditable == NULL)
		ImGui::PopStyleColor();

	ImGui::PopStyleColor(3);

	ImGui::End();
}


int ObjectLocalList::isOpen()
{
	return isVisible;
}