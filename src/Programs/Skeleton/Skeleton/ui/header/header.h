#ifndef HEADER_H
#define HEADER_H

#include "../../ImGui/imgui.h"

class Header {
private:
	enum LocalList {
		NONE, FILE, EDIT, VIEW, LAYOUT
	};

	static int currentLocalList;
	static ImVec2 localListPos;

public:
	static const int HeightInPixels = 35;

	static void render();
};

#endif