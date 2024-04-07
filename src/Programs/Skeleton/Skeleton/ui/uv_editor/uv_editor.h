#ifndef UV_EDITOR_UI_H
#define UV_EDITOR_UI_H

#include "../../framework.h"

class UVEditor {
public:
	static OPENFILENAMEA ofn;

	static void render(vec2 bottomLeft, vec2 topRight);
};

#endif