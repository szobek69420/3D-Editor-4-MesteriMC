#include "uv_editor.h"

#include "../../ImGui/imgui.h"
#include "../../System/system.h"

OPENFILENAMEA UVEditor::ofn;

void UVEditor::render(vec2 bottomLeft, vec2 topRight)
{
	if (topRight.x - bottomLeft.x < 200)
		return;

	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	ImGui::SetNextWindowBgAlpha(0);
	ImGui::SetNextWindowPos(ImVec2(bottomLeft.x + 0.5f * (topRight.x - bottomLeft.x) - 100, windowHeight-topRight.y));
	ImGui::SetNextWindowSize(ImVec2(200, 20));
	ImGui::Begin("uv_editor_header", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if (ImGui::Button("Import"))
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
		}
	}
	ImGui::End();
}