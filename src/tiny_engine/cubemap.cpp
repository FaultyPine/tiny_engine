//#include "pch.h"
#include "cubemap.h"


// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
Cubemap LoadCubemap(const std::vector<const char*>& facesPaths, TextureProperties props) {
    u32 textureID;
    GLCall(glGenTextures(1, &textureID));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    s32 width, height, nrChannels;
    for (u32 i = 0; i < facesPaths.size(); i++) {
        u8 *data = LoadImageData(facesPaths[i], &width, &height, &nrChannels);
        if (!data) {
            LOG_ERROR("Cubemap texture failed to load at path: %s", facesPaths[i]);
            TINY_ASSERT(false);
        }
        else {
            GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (s32)props.texFormat, width, height, 0, (s32)props.imgFormat, (s32)props.imgDataType, data));
        }
        free(data);
    }

    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (s32)props.texWrapMode));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (s32)props.texWrapMode));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (s32)props.texWrapMode));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, (s32)props.minFilter));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (s32)props.magFilter));
    GLCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

    // unbind
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    
    LOG_INFO("Loaded cubemaps: ");
    for (const char* s : facesPaths) LOG_INFO("%s, ", s);

    Cubemap ret;
    ret.id = textureID;
    return ret;
}
