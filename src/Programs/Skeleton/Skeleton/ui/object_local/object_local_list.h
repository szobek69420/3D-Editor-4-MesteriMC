#ifndef OBJECT_LOCAL_LIST_H
#define OBJECT_LOCAL_LIST_H

#include "../../framework.h"

class ObjectLocalList {
public:
	static const int width = 120, height = 120;

	static void open(vec2 mousePosInScreenSpace);
	static void close();

	static void render(vec2 bottomLeftInScreenSpace, vec2 topRightInScreenSpace);

	static int isOpen();
};

#endif