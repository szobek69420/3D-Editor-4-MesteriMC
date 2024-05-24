#ifndef EXPORT_H
#define EXPORT_H

class Exporter {
private:

public:
	static void exportEditable(const char* filePath, void* parents, unsigned int parentCount);
};

#endif
