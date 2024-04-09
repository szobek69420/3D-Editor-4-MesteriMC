#include "system.h"

int System::mousePosX = 0;
int System::mousePosY = 0;

int System::windowWidth = 600;
int System::windowHeight = 600;

void System::getMousePosition(int* x, int* y)
{
	*x = System::mousePosX;
	*y = System::mousePosY;
}

void System::setMousePosition(int x, int y)
{
	System::mousePosX = x;
	System::mousePosY = y;
}


void System::getWindowSize(int* width, int* height)
{
	*width = System::windowWidth;
	*height = System::windowHeight;
}

void System::setWindowSize(int width, int height)
{
	System::windowWidth = width;
	System::windowHeight = height;
}

vec2 System::convertScreenToGl(vec2 screenCoords)
{
	screenCoords.y = System::windowHeight - screenCoords.y;
	return screenCoords;
}