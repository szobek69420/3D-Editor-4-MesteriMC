#include "uv_editor.h"

#include "../../ImGui/imgui.h"
#include "../../System/system.h"
#include "../../Editable/editable.h"
#include "../../TextureLoader/texture_loader.h"

OPENFILENAMEA UVEditor::ofn;
extern Editable* selectedEditable;

void UVEditor::render(vec2 bottomLeft, vec2 topRight)
{
	if (topRight.x - bottomLeft.x < 300)
		return;

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui::SetNextWindowBgAlpha(0);
	ImGui::SetNextWindowPos(ImVec2(bottomLeft.x + 0.5f * (topRight.x - bottomLeft.x) - 100, topRight.y));
	ImGui::SetNextWindowSize(ImVec2(300, 20));
	ImGui::Begin("uv_editor_header", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	
	ImGui::SameLine();
	if (ImGui::Button("Import")&& selectedEditable != NULL)
	{
		OPENFILENAMEA open;
		CHAR szFile[MAX_PATH] = { 0 }; // Initialize buffer for file path

		ZeroMemory(&open, sizeof(open));
		open.lStructSize = sizeof(OPENFILENAMEA);
		open.lpstrFilter = "Textures\0*.jpg;*.jpeg;*.gif;*.png;*.bmp\0\0";
		open.nFileOffset = 1;
		open.lpstrFile = szFile; // Assign buffer to store file path
		open.nMaxFile = MAX_PATH;
		open.lpstrTitle = "Choose texture..";
		open.Flags = OFN_FILEMUSTEXIST;

		// Display the Open dialog box
		if (GetOpenFileNameA(&open) == TRUE) {
			UVEditor::ofn = open;
			selectedEditable->setAlbedo(TextureLoader::load(ofn.lpstrFile, GL_LINEAR, 69), ofn.lpstrFile);
		}
	}


	ImGui::SameLine(70);
	ImGui::SetNextItemWidth(200);
	if (selectedEditable != NULL)
		ImGui::Text("%.19s",selectedEditable->getAlbedoPath());
	else
		ImGui::Text("no texture");


	ImGui::SameLine(190);
	ImGui::SetNextItemWidth(30);
	if (selectedEditable != NULL && ImGui::Button("X"))
		selectedEditable->setAlbedo(0, "");
	ImGui::End();
}
