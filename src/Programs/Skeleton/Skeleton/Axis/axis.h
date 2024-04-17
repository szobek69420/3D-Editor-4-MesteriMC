#ifndef AXIS_H
#define AXIS_H

#include "../framework.h"
#include "../Camera/camera.h"

class Axis {
public:
	enum Direction {
		DIR_X=-69, DIR_Y=0, DIR_Z=69
	};

	static void initialize();
	static void deinitialize();

	static void render(Axis::Direction direction, const Camera& cum, vec3 center, const vec2& bottomLeft, const vec2& topRight);
};

#endif