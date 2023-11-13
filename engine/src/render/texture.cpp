//#include "pch.h"
#include "texture.h"
#include "shader.h"
#include "tiny_ogl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

enum class TextureProperties::TexWrapMode : s32 {
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    REPEAT = GL_REPEAT,
    MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE,
};
enum class TextureProperties::TexMinFilter : s32 {
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
};
enum class TextureProperties::TexMagFilter : s32 {
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR,
};
enum class TextureProperties::TexFormat : s32 {
    RGB = GL_RGB,
    RGBA = GL_RGBA,
    RG = GL_RG,
    RED = GL_RED,
    DEPTH_STENCIL = GL_DEPTH_STENCIL,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
};
enum class TextureProperties::ImageFormat : s32 {
    RGB = GL_RGB,
    RGBA = GL_RGBA,
};
enum class TextureProperties::ImageDataType : s32 {
    BYTE = GL_BYTE,
    UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    SHORT = GL_SHORT,
    UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
    INT = GL_INT,
    UNSIGNED_INT = GL_UNSIGNED_INT,
};

/// DEFAULTS: Wrap = mirrored repeat Min = lin_mip_lin Mag = lin TexFormat = RGBA ImgFormat = RGBA ImgData = UnsignedByte
TextureProperties TextureProperties::RGBA_LINEAR() {
    TextureProperties texProps;
    texProps.texWrapMode = TextureProperties::TexWrapMode::MIRRORED_REPEAT;
    texProps.minFilter = TextureProperties::TexMinFilter::LINEAR_MIPMAP_LINEAR;
    texProps.magFilter = TextureProperties::TexMagFilter::LINEAR;
    texProps.texFormat = TextureProperties::TexFormat::RGBA;
    texProps.imgFormat = TextureProperties::ImageFormat::RGBA;
    texProps.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
    return texProps;
}
TextureProperties TextureProperties::RGBA_NEAREST() {
    TextureProperties texProps = RGBA_LINEAR();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    return texProps;
}
TextureProperties TextureProperties::RGB_LINEAR() {
    TextureProperties texProps = RGBA_LINEAR();
    texProps.imgFormat = TextureProperties::ImageFormat::RGB;
    texProps.texFormat = TextureProperties::TexFormat::RGB;
    return texProps;
}
TextureProperties TextureProperties::RGB_NEAREST() {
    TextureProperties texProps = RGB_LINEAR();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    return texProps;
}
TextureProperties TextureProperties::None() {
    TextureProperties texProps = RGBA_LINEAR();
    texProps.isNone = true;
    return texProps;
}

const char* GetTexMatTypeString(TextureMaterialType type) {
    switch (type) {
        case DIFFUSE: return "tex_diffuse";
        case SPECULAR: return "tex_specular";
        case AMBIENT: return "tex_ambient";
        case NORMAL: return "tex_normal";
        case ROUGHNESS: return "tex_roughness";
        case EMISSION: return "tex_emission";
        case OTHER: return "tex_other";

        default: return "tex_unknown";
    }
}


Texture::Texture(u32 id, u32 type, u32 width, u32 height)
{
    this->id = id;
    this->type = type;
    this->width = width;
    this-> height = height;
}

void Texture::Delete() { GLCall(glDeleteTextures(1, (const GLuint*)&id)); }
void Texture::bindUnit(u32 textureUnit, u32 id, u32 textureType) {
    TINY_ASSERT(textureType != 0 && "Attempted to bind texture with invalid type!");
    // bind id to given texture unit
    activate(textureUnit);
    GLCall(glBindTexture(textureType, id));
}

void Texture::activate(u32 textureUnit) { GLCall(glActiveTexture(GL_TEXTURE0 + textureUnit)); }

// https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glPixelStorei.xhtml
void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment) {
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, offsetX);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, offsetY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


u8* LoadImageData(const std::string& imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically) {
    stbi_set_flip_vertically_on_load(shouldFlipVertically);
    u8* data = stbi_load(imgPath.c_str(), width, height, numChannels, 0);
    if (!data)
    {
        LOG_ERROR("Failed to load image at %s", imgPath.c_str());
    }
    return data;
}


Texture GenTextureFromImg(u8* imgData, u32 width, u32 height, TextureProperties props) {
    u32 texture = 0;
    if (imgData) {
        GLCall(glGenTextures(1, &texture));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        // wrapping mode
        //              tex target     tex wrap axis      tex wrap mode
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (s32)props.texWrapMode));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (s32)props.texWrapMode));
        // filter mode
        //               apply taget to minified or magnified textures
        //              tex target         ^                  tex filter mode & mipmap filter mode
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (s32)props.minFilter));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (s32)props.magFilter)); // don't specify mipmap filtering on upscaling, since it only happens when you downscale
        // tex target, mipmap level, tex format, width, height, 0 (legacy stuff), format of src img, datatype of src img, actual img data
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, (s32)props.texFormat, width, height, 0, (s32)props.imgFormat, (s32)props.imgDataType, imgData));
        GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else {
        return Texture();
    }
    Texture ret;
    ret.id = texture;
    ret.height = (u32)height;
    ret.width = (u32)width;
    ret.type = GL_TEXTURE_2D;
    return ret;
}

// Note: May crash with invalid path/image. Should this return an optional?
Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props, 
                    bool flipVertically) {
    s32 width, height, numChannels;
    u8* data = LoadImageData(imgPath, &width, &height, &numChannels, flipVertically);
    // if we didn't specify texture properties, fetch them automatically
    if (props.isNone) {
        if (numChannels == 3) {
            props = TextureProperties::RGB_LINEAR();
        }
        else if (numChannels == 4) {
            props = TextureProperties::RGBA_LINEAR();
        }
        else if (numChannels == 1) {
            // for L8 images   *UNTESTED*
            props = TextureProperties::RGB_LINEAR();
            props.texFormat = TextureProperties::TexFormat::RED;
            props.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
        }
        else {
            TINY_ASSERT(false && "Unknown number of channels in image!");
        }
    }
    Texture ret = GenTextureFromImg(data, width, height, props);
    if (ret.id == 0) {
        // invalid texture
        LOG_ERROR("Couldn't load %s", imgPath);
        TINY_ASSERT(false && "failed to load texture!");
    }
    ret.texpath = imgPath;
    stbi_image_free(data);
    LOG_INFO("Loaded texture %s  channels: %i", ret.texpath.c_str(), numChannels);
    return ret;
}


void Material::SetShaderUniforms(const Shader& shader, u32 matIdx) const {
    s32 matPropIdx = 0;
   #define SET_MATERIAL_UNIFORMS(matVar) \
        shader.setUniform(TextFormat("materials[%i].%s.useSampler", matIdx, #matVar), matVar.hasTexture); \
        if (matVar.hasTexture) { \
            shader.TryAddSampler(matVar.texture, TextFormat("materials[%i].%s.tex", matIdx, #matVar)); \
        } \
        shader.setUniform(TextFormat("materials[%i].%s.color", matIdx, #matVar), matVar.color)

    SET_MATERIAL_UNIFORMS(diffuseMat);
    SET_MATERIAL_UNIFORMS(ambientMat);
    SET_MATERIAL_UNIFORMS(specularMat);
    SET_MATERIAL_UNIFORMS(normalMat);
    SET_MATERIAL_UNIFORMS(shininessMat);
    SET_MATERIAL_UNIFORMS(emissiveMat);
    shader.setUniform("useNormalMap", normalMat.hasTexture);
}