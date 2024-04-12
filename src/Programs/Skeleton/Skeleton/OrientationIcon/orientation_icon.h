#ifndef ORIENTATION_ICON_H
#define ORIENTATION_ICON_H

#include "../framework.h"
#include "../Camera/camera.h"

class OrientationIcon {
public:
	static void initialize();
	static void deinitialize();

	static void render(const Camera& cum, vec2 bottomLeft, vec2 topRight);
};

#endif