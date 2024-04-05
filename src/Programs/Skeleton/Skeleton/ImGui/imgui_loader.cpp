#include "imgui_loader.h"

#include "../framework.h"

#include <GL/glew.h>		// must be downloaded
#include <GL/freeglut.h>	// must be downloaded unless you have an Apple

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

#include "imstb_truetype.h"


void* ImGuiLoader::font = nullptr;

void ImGuiLoader::initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer bindings
	ImGui_ImplGLUT_Init();
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();

	//import font
	ImGuiIO& io = ImGui::GetIO();
	ImGuiLoader::font = io.Fonts->AddFontFromFileTTF("./assets/Monocraft.ttf", 12.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	IM_ASSERT((ImFont*)font != NULL);
	io.FontDefault = io.Fonts->Fonts.back();
}

void ImGuiLoader::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}

