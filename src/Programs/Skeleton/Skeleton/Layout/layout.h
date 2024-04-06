#ifndef LAYOUT_H
#define LAYOUT_H

#include <vector>

typedef int layout_t;

class Layout {
private:
	static std::vector<layout_t> panels;

public:
	static const layout_t OBJECT = 1;
	static const layout_t UV = 2;
	static const layout_t OBJECT_UV = 3;

	static const std::vector<layout_t>& getLayout();
	static void setLayout(layout_t preset);
	static void setLayout(const layout_t* panels, unsigned int count);
};


#endif