#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

class TextureLoader {
public:
	static unsigned int load(const char* pathToTexture, int filterType, int flipVertically);
};

#endif