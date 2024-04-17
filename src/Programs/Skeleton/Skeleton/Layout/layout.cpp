#include "layout.h"

#include "../System/system.h"
#include "../ui/header/header.h"

#include "../ImGui/imgui.h"

#include <string.h>


static layout_t layoutPresets_object[] = { Layout::OBJECT };
static layout_t layoutPresets_uv[] = { Layout::UV };
static layout_t layoutPresets_objectUv[] = { Layout::OBJECT, Layout::UV };

std::vector<struct LayoutPanelInfo> Layout::panels = std::vector<struct LayoutPanelInfo>();

const std::vector<struct LayoutPanelInfo>& Layout::getLayout()
{
	return Layout::panels;
}

void Layout::setLayout(Layout::Preset preset)
{
	switch (preset)
	{
		case Layout::Preset::Object:
			Layout::setLayout(layoutPresets_object, 1);
			break;

		case Layout::Preset::Uv:
			Layout::setLayout(layoutPresets_uv, 1);
			break;

		case Layout::Preset::ObjectUv:
			Layout::setLayout(layoutPresets_objectUv, 2);
			break;
	}
}
void Layout::setLayout(const layout_t* panels, unsigned int count)
{
	if (count == 0)
	{
		Layout::panels.clear();
		return;
	}

	int width, height;
	System::getWindowSize(&width, &height);
	width -= 200;	

	Layout::panels.clear();
	for (int i = 0; i < count; i++)
	{
		vec2 top, bottom;
		bottom.x = i * (width / count);
		bottom.y = height;
		top.x = (i + 1) * (width / count);
		top.y = Header::HeightInPixels;

		Layout::panels.push_back(LayoutPanelInfo::create(panels[i], bottom, top));
	}
}

void Layout::refresh()
{
	if (Layout::panels.size() == 0)
		return;

	layout_t* temp = new layout_t[Layout::panels.size()];
	for (int i = 0; i < Layout::panels.size(); i++)
		temp[i] = Layout::panels[i].type;

	Layout::setLayout(temp, Layout::panels.size());

	delete[] temp;
}

int Layout::getLayoutBounds(layout_t type, vec2* bottomLeft, vec2* topRight)
{
	for (int i = 0; i < Layout::panels.size(); i++)
	{
		if (Layout::panels[i].type != type)
			continue;

		*bottomLeft = Layout::panels[i].bottomLeft;
		*topRight = Layout::panels[i].topRight;
		return 69;
	}
	return 0;
}

layout_t Layout::getLayoutByMousePos(int pX, int pY)
{
	for (int i = 0; i < Layout::panels.size(); i++)
	{
		if (pX<Layout::panels[i].bottomLeft.x || pX>Layout::panels[i].topRight.x || pY>Layout::panels[i].bottomLeft.y || pY<Layout::panels[i].topRight.y)
			continue;

		return Layout::panels[i].type;
	}
	return Layout::NONE;
}

void Layout::renderImGUI()
{
	if (Layout::panels.size() < 1)
		return;

	for (int i = 0; i < Layout::panels.size(); i++)
	{
		static char windowName[20];
		sprintf_s(windowName, 20, "layout_window_%d", i);

		ImGui::SetNextWindowPos(ImVec2(Layout::panels[i].bottomLeft.x, Layout::panels[i].topRight.y));
		ImGui::SetNextWindowSize(ImVec2(Layout::panels[i].topRight.x-Layout::panels[i].bottomLeft.x, Layout::panels[i].bottomLeft.y - Layout::panels[i].topRight.y));
	}
}