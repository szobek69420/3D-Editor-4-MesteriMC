#ifndef IMGUI_LOADER_H
#define IMGUI_LOADER_H

class ImGuiLoader {
public:
	static void initialize();
	static void destroy();

private:
	static void* font;
};

#endif