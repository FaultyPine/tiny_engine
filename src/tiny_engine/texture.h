#ifndef TEXTURE_H
#define TEXTURE_H

#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
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
std::string GetTexMatTypeString(TextureMaterialType type) {
    switch (type) {
        case DIFFUSE: return "tex_diffuse";
        case SPECULAR: return "tex_specular";
        case NORMAL: return "tex_normal";
        case ROUGHNESS: return "tex_roughness";
        case EMISSION: return "tex_emission";
        case OTHER: return "tex_other";

        default: return "tex_unknown";
    }
}

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
};


// TODO: use optional type for return value
Texture LoadTexture(const char* imgPath, 
                    TextureProperties props, 
                    TextureMaterialType texType = TextureMaterialType::OTHER, 
                    bool flipVertically = false) {
    stbi_set_flip_vertically_on_load(flipVertically);  
    s32 width, height, nrChannels;
    u8* data = stbi_load(imgPath, &width, &height, &nrChannels, 0);
    u32 texture = 0; // good default? 
    if (data) {
        std::cout << "Loaded img " << imgPath << "\n";
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // wrapping mode
        //              tex target     tex wrap axis      tex wrap mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (s32)props.texWrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (s32)props.texWrapMode);
        // filter mode
        //               apply taget to minified or magnified textures
        //              tex target         ^                  tex filter mode & mipmap filter mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (s32)props.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (s32)props.magFilter); // don't specify mipmap filtering on upscaling, since it only happens when you downscale
        // tex target, mipmap level, tex format, width, height, 0 (legacy stuff), format of src img, datatype of src img, actual img data
        glTexImage2D(GL_TEXTURE_2D, 0, (s32)props.texFormat, width, height, 0, (s32)props.imgFormat, (s32)props.imgDataType, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture " << imgPath << "\n";
    }
    stbi_image_free(data);
    Texture ret;
    ret.id = texture;
    ret.type = texType;
    return ret;
}

#endif