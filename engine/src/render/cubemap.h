#ifndef TINY_CUBEMAP_H
#define TINY_CUBEMAP_H

//#include "pch.h"
#include "tiny_defines.h"
#include "texture.h"
#include <vector>

struct Cubemap {
    s32 id = 0;
};

TAPI Cubemap LoadCubemap(const std::vector<const char*>& facesPaths, TextureProperties props);

#endif