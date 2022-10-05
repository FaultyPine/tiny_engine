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

    /// DEFAULTS: Wrap = mirrored repeat Min = lin_mip_lin Mag = lin TexFormat = RGBA ImgFormat = RGBA ImgData = UnsignedByte
    static TextureProperties Default() {
        TextureProperties texProps;
        // defaults
        texProps.texWrapMode = TextureProperties::TexWrapMode::MIRRORED_REPEAT;
        texProps.minFilter = TextureProperties::TexMinFilter::LINEAR_MIPMAP_LINEAR;
        texProps.magFilter = TextureProperties::TexMagFilter::LINEAR;
        texProps.texFormat = TextureProperties::TexFormat::RGBA;
        texProps.imgFormat = TextureProperties::ImageFormat::RGBA;
        texProps.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
        return texProps;
    }
};

struct Texture {
    u32 id = 0;
    f32 width, height = 0.0;
    TextureMaterialType type = TextureMaterialType::DIFFUSE;

    Texture() { id = 0; type = TextureMaterialType::DIFFUSE; width = 0.0; height = 0.0; }
    void bind() const { glBindTexture(GL_TEXTURE_2D, id); }
};

struct Material {
    glm::vec3 diffuseColor;
    bool hasTexture;
    std::string diffuseTexPath;
    Texture diffuseTex;

    Material(glm::vec3 diffuseCol) { diffuseColor = diffuseCol; }
    Material(glm::vec3 diffuseCol, const std::string& texName) { diffuseColor = diffuseCol; diffuseTexPath = texName; }
};

void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment = 4);

u8* LoadImageData(const char* imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically = false);

Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props, 
                    TextureMaterialType texType = TextureMaterialType::OTHER, 
                    bool flipVertically = false);
Texture GenTextureFromImg(u8* imgData, u32 width, u32 height, 
                    TextureProperties props, TextureMaterialType texType = TextureMaterialType::DIFFUSE);

#endif