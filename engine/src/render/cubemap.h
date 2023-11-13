#ifndef TINY_CUBEMAP_H
#define TINY_CUBEMAP_H

//#include "pch.h"
#include "tiny_defines.h"
#include "texture.h"
#include <vector>

struct Cubemap {
    s32 id = 0;
    // width and height of each face
    // NOTE: opengl requires all faces of a cubemap to have the same resolution/size
    u32 width = 0;
    u32 height = 0;
};

TAPI Cubemap LoadCubemap(const std::vector<const char*>& facesPaths, TextureProperties props);
#endif