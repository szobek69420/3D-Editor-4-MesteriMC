#ifndef GRID_H
#define GRID_H

#include "../framework.h"
#include "../Camera/camera.h"

class Grid {

	static vec3 colour;
	static float step;

public:
	static void initialize();
	static void deinitialize();

	static void render(unsigned int count, const vec2& bottomLeft, const vec2& topRight, const Camera& cum, float start);//a start, hogy mekkora offsetrol induljanak a vonalak, a step pedig a leptek

	static void setStepSize(float stepSize);
	static void setColour(float r, float g, float b);
};

#endif