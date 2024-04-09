#ifndef SYSTEM_H
#define SYSTEM_H

#include "../framework.h"

class System {
private:
	static int mousePosX;
	static int mousePosY;

	static int windowWidth, windowHeight;

public:
	static void getMousePosition(int* x, int* y);
	static void setMousePosition(int x, int y);

	static void getWindowSize(int* width, int* height);
	static void setWindowSize(int width, int height);

	static vec2 convertScreenToGl(vec2 screenCoords);
};

#endif