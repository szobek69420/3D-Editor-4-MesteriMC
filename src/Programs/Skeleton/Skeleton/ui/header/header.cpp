#include "header.h"

#include <vector>

#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_loader.h"

#include "../../System/system.h"

#include "../../Layout/layout.h"

#include "../../Editable/editable.h"

//externals
void endOperation(int discard);

extern Editable* selectedEditable;
extern std::vector<unsigned int> selectedVertexIDs;
extern int showVertices;

//others
int Header::currentLocalList = Header::LocalList::NONE;
ImVec2 Header::localListPos = ImVec2();


void Header::render()
{
	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(windowWidth, Header::HeightInPixels));
	ImGui::Begin("header", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar);
	
	if (ImGui::Button("File"))
	{
		currentLocalList = Header::LocalList::FILE;

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		localListPos = ImVec2(buttonMin.x + windowPos.x, buttonMax.y + windowPos.y);
	}
	ImGui::SameLine(0);
	if (ImGui::Button("Edit"))
	{
		currentLocalList = Header::LocalList::EDIT;

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		localListPos = ImVec2(buttonMin.x + windowPos.x, buttonMax.y + windowPos.y);
	}
	ImGui::SameLine();
	if (ImGui::Button("View"))
	{
		currentLocalList = Header::LocalList::VIEW;

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		localListPos = ImVec2(buttonMin.x + windowPos.x, buttonMax.y + windowPos.y);
	}
	ImGui::SameLine();
	if (ImGui::Button("Layout"))
	{
		currentLocalList = Header::LocalList::LAYOUT;

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		localListPos = ImVec2(buttonMin.x + windowPos.x, buttonMax.y + windowPos.y);
	}

	ImGui::End();


	switch (currentLocalList)
	{
	case Header::LocalList::FILE:
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) //has the user clicked out of the local list
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (mousePos.x<localListPos.x || mousePos.y<localListPos.y || mousePos.x>localListPos.x + 100 || mousePos.y>localListPos.y + 100)
			{
				currentLocalList = Header::LocalList::NONE;
				break;
			}
		}

		ImGui::SetNextWindowPos(localListPos);
		ImGui::SetNextWindowSize(ImVec2(100, 100));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::Begin("header_file", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));
		
		ImGui::Button("New");
		ImGui::Button("Import");
		ImGui::Button("Save");
		ImGui::Button("Save as...");

		ImGui::PopStyleColor(3);

		ImGui::End();

		ImGui::PopStyleColor();
		break;

	case Header::LocalList::EDIT:
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) //has the user clicked out of the local list
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (mousePos.x<localListPos.x || mousePos.y<localListPos.y || mousePos.x>localListPos.x + 100 || mousePos.y>localListPos.y + 100)
			{
				currentLocalList = Header::LocalList::NONE;
				break;
			}
		}

		ImGui::SetNextWindowPos(localListPos);
		ImGui::SetNextWindowSize(ImVec2(100, 60));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::Begin("header_edit", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));

		if (ImGui::Button("Object mode"))
		{
			currentLocalList = Header::LocalList::NONE;
			endOperation(69);
			selectedVertexIDs.clear();
			showVertices = 0;
		}
		if (ImGui::Button("Edit mode"))
		{
			currentLocalList = Header::LocalList::NONE;
			endOperation(69);
			selectedVertexIDs.clear();
			if (selectedEditable != NULL)
				showVertices = 1;
		}

		ImGui::PopStyleColor(3);

		ImGui::End();

		ImGui::PopStyleColor();
		break;

	case Header::LocalList::VIEW:
		
		break;

	case Header::LocalList::LAYOUT:
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) //has the user clicked out of the local list
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (mousePos.x<localListPos.x || mousePos.y<localListPos.y || mousePos.x>localListPos.x + 100 || mousePos.y>localListPos.y + 100)
			{
				currentLocalList = Header::LocalList::NONE;
				break;
			}
		}

		ImGui::SetNextWindowPos(localListPos);
		ImGui::SetNextWindowSize(ImVec2(100, 80));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::Begin("header_layout", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));

		if (ImGui::Button("3D"))
			Layout::setLayout(Layout::Preset::Object);
		if(ImGui::Button("Texture"))
			Layout::setLayout(Layout::Preset::Uv);
		if(ImGui::Button("3D | Texture"))
			Layout::setLayout(Layout::Preset::ObjectUv);

		ImGui::PopStyleColor(3);

		ImGui::End();

		ImGui::PopStyleColor();
		break;
	}
}