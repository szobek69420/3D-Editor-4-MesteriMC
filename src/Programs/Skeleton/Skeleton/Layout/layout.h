#ifndef LAYOUT_H
#define LAYOUT_H

#include <vector>

#include "../framework.h"

typedef int layout_t;

struct LayoutPanelInfo {
	layout_t type;
	vec2 bottomLeft;
	vec2 topRight;

	static struct LayoutPanelInfo create(layout_t type, vec2 bottomLeft, vec2 topRight)
	{
		struct LayoutPanelInfo lpi;
		lpi.type = type;
		lpi.bottomLeft = bottomLeft;
		lpi.topRight = topRight;
		return lpi;
	}
};

class Layout {
private:
	static std::vector<struct LayoutPanelInfo> panels;

public:
	enum class Preset {
		Object,
		Uv,
		ObjectUv
	};

	static const layout_t NONE = 0;
	static const layout_t OBJECT = 1;
	static const layout_t UV = 2;
	

	static const std::vector<struct LayoutPanelInfo>& getLayout();
	static void setLayout(Preset preset);
	static void setLayout(const layout_t* panels, unsigned int count);
	static void refresh();//should be called when window size changes

	static void renderImGUI();

	static int getLayoutBounds(layout_t type, vec2* bottomLeft, vec2* topRight);//returns 0, if there is no such layout_t displayed

	static layout_t getLayoutByMousePos(int pX, int pY);
};


#endif