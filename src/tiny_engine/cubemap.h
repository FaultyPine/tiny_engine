#ifndef TINY_CUBEMAP_H
#define TINY_CUBEMAP_H

#include "pch.h"
#include "texture.h"

struct Cubemap {
    s32 id = 0;
};

Cubemap LoadCubemap(const std::vector<const char*>& facesPaths, TextureProperties props);
f32* GetCubemapCubeVertices(u32* numElements);

#endif