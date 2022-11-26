#ifndef TINY_TEXTURE_H
#define TINY_TEXTURE_H

#include "pch.h"
#include "stb_image.h"
#include "tiny_engine/shader.h"

enum TextureMaterialType {
    DIFFUSE = 0,
    AMBIENT,
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
    bool isNone = false;

    /// DEFAULTS: Wrap = mirrored repeat Min = lin_mip_lin Mag = lin TexFormat = RGBA ImgFormat = RGBA ImgData = UnsignedByte
    static TextureProperties RGBA_LINEAR() {
        TextureProperties texProps;
        texProps.texWrapMode = TextureProperties::TexWrapMode::MIRRORED_REPEAT;
        texProps.minFilter = TextureProperties::TexMinFilter::LINEAR_MIPMAP_LINEAR;
        texProps.magFilter = TextureProperties::TexMagFilter::LINEAR;
        texProps.texFormat = TextureProperties::TexFormat::RGBA;
        texProps.imgFormat = TextureProperties::ImageFormat::RGBA;
        texProps.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
        return texProps;
    }
    static TextureProperties RGBA_NEAREST() {
        TextureProperties texProps = RGBA_LINEAR();
        texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
        texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
        return texProps;
    }
    static TextureProperties RGB_LINEAR() {
        TextureProperties texProps = RGBA_LINEAR();
        texProps.imgFormat = TextureProperties::ImageFormat::RGB;
        texProps.texFormat = TextureProperties::TexFormat::RGB;
        return texProps;
    }
    static TextureProperties RGB_NEAREST() {
        TextureProperties texProps = RGB_LINEAR();
        texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
        texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
        return texProps;
    }
    static TextureProperties None() {
        TextureProperties texProps = RGBA_LINEAR();
        texProps.isNone = true;
        return texProps;
    }
};

struct Texture {
    // when we pass texture id to our shader, it needs to use the glUniform1i (<- signed) which is why this is s32 not u32
    s32 id = 0; // this is the actual opengl texture id, the rest of these fields are just extra info for convinience
    f32 width, height = 0.0;
    TextureMaterialType type = TextureMaterialType::DIFFUSE;
    std::string texpath;

    Texture() {}
    Texture(s32 id) { Texture(); this->id = id; }
    void bind() const { GLCall(glBindTexture(GL_TEXTURE_2D, id)); }
    static void activate(u32 textureUnit) { GLCall(glActiveTexture(GL_TEXTURE0 + textureUnit)); }
};

struct MaterialProp {
    bool hasTexture = false;
    glm::vec4 color = glm::vec4(1);
    Texture texture;
    MaterialProp() {}
};
struct Material {
    MaterialProp diffuseMat;
    MaterialProp ambientMat;
    MaterialProp specularMat;
    f32 shininess;
    std::string name = "";

    Material(){}
    Material(MaterialProp diffuse, MaterialProp ambient, MaterialProp specular, std::string name) {
        diffuseMat = diffuse;
        ambientMat = ambient;
        specularMat = specular;
        this->name = name;
    }
    void SetShaderUniforms(const Shader& shader) const {
        #define MAT_DIFFUSE   0
        #define MAT_AMBIENT   1
        #define MAT_SPECULAR  2
        shader.use();

        #define SET_MATERIAL_UNIFORMS(matIdx, matVar) \
            shader.setUniform(TextFormat("materials[%i].useSampler", matIdx), matVar.hasTexture); \
            glActiveTexture(GL_TEXTURE0 + matIdx); \
            matVar.texture.bind(); \
            shader.setUniform(TextFormat("materials[%i].tex", matIdx), matVar.texture.id); \
            shader.setUniform(TextFormat("materials[%i].color", matIdx), matVar.color)

        // opengl error on setUniform(materials[0].tex)
        SET_MATERIAL_UNIFORMS(MAT_DIFFUSE, diffuseMat);
        SET_MATERIAL_UNIFORMS(MAT_AMBIENT, ambientMat);
        SET_MATERIAL_UNIFORMS(MAT_SPECULAR, specularMat);
        shader.setUniform("shininess", shininess);
    }
};

void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment = 4);

u8* LoadImageData(const char* imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically = false);

Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props = TextureProperties::None(), 
                    TextureMaterialType texType = TextureMaterialType::OTHER, 
                    bool flipVertically = false);
Texture GenTextureFromImg(u8* imgData, u32 width, u32 height, 
                    TextureProperties props, TextureMaterialType texType = TextureMaterialType::DIFFUSE);

#endif