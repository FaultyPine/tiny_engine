#ifndef TINY_TEXTURE_H
#define TINY_TEXTURE_H

#include "pch.h"
#include "stb_image.h"

enum TextureMaterialType {
    DIFFUSE = 0,
    SPECULAR,
    NORMAL,
    ROUGHNESS,
    EMISSION,
    OTHER,

    NUM_TYPES,
};
std::string GetTexMatTypeString(TextureMaterialType type);

struct TextureProperties {
    enum class TexWrapMode : s32 {
        CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
        CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
        MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
        REPEAT = GL_REPEAT,
        MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE,
    };
    enum class TexMinFilter : s32 {
        NEAREST = GL_NEAREST,
        LINEAR = GL_LINEAR,
        NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
    };
    enum class TexMagFilter : s32 {
        NEAREST = GL_NEAREST,
        LINEAR = GL_LINEAR,
    };
    enum class TexFormat : s32 {
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        RG = GL_RG,
        RED = GL_RED,
        DEPTH_STENCIL = GL_DEPTH_STENCIL,
        DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    };
    enum class ImageFormat : s32 {
        RGB = GL_RGB,
        RGBA = GL_RGBA,
    };
    enum class ImageDataType : s32 {
        BYTE = GL_BYTE,
        UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
        SHORT = GL_SHORT,
        UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
        INT = GL_INT,
        UNSIGNED_INT = GL_UNSIGNED_INT,
    };

    TexWrapMode texWrapMode;
    TexMinFilter minFilter;
    TexMagFilter magFilter;
    TexFormat texFormat;
    ImageFormat imgFormat;
    ImageDataType imgDataType;
};

struct Texture {
    u32 id;
    TextureMaterialType type;

    void bind() { glBindTexture(GL_TEXTURE_2D, id); }
};

struct Material {
    glm::vec3 diffuseColor;
    bool hasTexture;
    std::string diffuseTexPath;
    Texture diffuseTex;

    Material(glm::vec3 diffuseCol) { diffuseColor = diffuseCol; }
    Material(glm::vec3 diffuseCol, const std::string& texName) { diffuseColor = diffuseCol; diffuseTexPath = texName; }
};


Texture LoadTexture(const char* imgPath, 
                    TextureProperties props, 
                    TextureMaterialType texType = TextureMaterialType::OTHER, 
                    bool flipVertically = false);

#endif