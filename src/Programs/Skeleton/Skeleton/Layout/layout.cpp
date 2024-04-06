#include "layout.h"


static layout_t layoutPresets_object[] = { Layout::OBJECT };
static layout_t layoutPresets_uv[] = { Layout::UV };
static layout_t layoutPresets_objectUv[] = { Layout::OBJECT, Layout::UV };

std::vector<layout_t> Layout::panels = std::vector<layout_t>(layoutPresets_object, layoutPresets_object+1);

const std::vector<layout_t>& Layout::getLayout()
{
	return Layout::panels;
}

void Layout::setLayout(layout_t preset)
{
	switch (preset)
	{
		case Layout::OBJECT:
			Layout::setLayout(layoutPresets_object, 1);
			break;

		case Layout::UV:
			Layout::setLayout(layoutPresets_uv, 1);
			break;

		case Layout::OBJECT_UV:
			Layout::setLayout(layoutPresets_objectUv, 2);
			break;
	}
}
void Layout::setLayout(const layout_t* panels, unsigned int count)
{
	Layout::panels.assign(panels, panels + count);
}