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
    //                  examples
    u32 texWrapMode; // GL_MIRRORED_REPEAT
    u32 minFilter;   // GL_LINEAR_MIPMAP_LINEAR
    u32 magFilter;   // GL_LINEAR
    u32 texFormat;   // GL_RGB
    u32 imgFormat;   // GL_RGB
    u32 imgDataType; // GL_UNSIGNED_BYTE
};
struct Texture {
    u32 id;
    TextureMaterialType type;
};


// TODO: use optional type for return value
Texture LoadTexture(const char* imgPath, TextureProperties props, TextureMaterialType texType = TextureMaterialType::OTHER, bool flipVertically = true) {
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, props.texWrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, props.texWrapMode);
        // filter mode
        //               apply taget to minified or magnified textures
        //              tex target         ^                  tex filter mode & mipmap filter mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, props.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, props.magFilter); // don't specify mipmap filtering on upscaling, since it only happens when you downscale
        // tex target, mipmap level, tex format, width, height, 0 (legacy stuff), format of src img, datatype of src img, actual img data
        glTexImage2D(GL_TEXTURE_2D, 0, props.texFormat, width, height, 0, props.imgFormat, props.imgDataType, data);
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