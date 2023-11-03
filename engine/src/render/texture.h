#ifndef TINY_TEXTURE_H
#define TINY_TEXTURE_H

//#include "pch.h"
#include "tiny_defines.h"
//#include "tiny_ogl.h"
#include "tiny_math.h"
#include <string>

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
const char* GetTexMatTypeString(TextureMaterialType type);
// TODO: get rid of the ogl usages in this header
struct TextureProperties {
    enum class TexWrapMode : s32;
    enum class TexMinFilter : s32;
    enum class TexMagFilter : s32;
    enum class TexFormat : s32;
    enum class ImageFormat : s32;
    enum class ImageDataType : s32;

    TexWrapMode texWrapMode;
    TexMinFilter minFilter;
    TexMagFilter magFilter;
    TexFormat texFormat;
    ImageFormat imgFormat;
    ImageDataType imgDataType;
    bool isNone = false;

    /// DEFAULTS: Wrap = mirrored repeat Min = lin_mip_lin Mag = lin TexFormat = RGBA ImgFormat = RGBA ImgData = UnsignedByte
    TAPI static TextureProperties RGBA_LINEAR();
    TAPI static TextureProperties RGBA_NEAREST();
    TAPI static TextureProperties RGB_LINEAR();
    TAPI static TextureProperties RGB_NEAREST();
    TAPI static TextureProperties None();
};

struct Texture {
    // when we pass texture id to our shader, it needs to use the glUniform1i (<- signed) which is why this is s32 not u32
    s32 id = 0; // this is the actual opengl texture id, the rest of these fields are just extra info for convinience
    f32 width, height = 0.0;
    std::string texpath = "";

    Texture() = default;
    Texture(s32 id) { Texture(); this->id = id; }
    void Delete();
    void bindUnit(u32 textureUnit) const;
    static void bindUnit(u32 textureUnit, u32 id);
    void bind() const;
    static void activate(u32 textureUnit);
};

struct MaterialProp {
    bool hasTexture = false;
    glm::vec4 color = glm::vec4(1);
    Texture texture = {};
    MaterialProp() = default;
    MaterialProp(glm::vec4 col) {
        color = col;
    }
    MaterialProp(const Texture& tex) {
        texture = tex;
    }
    void Delete() { texture.Delete(); }
};
struct Shader;
struct Material {
    MaterialProp diffuseMat = {};
    MaterialProp ambientMat = {};
    MaterialProp specularMat = {};
    MaterialProp normalMat = {};
    f32 shininess = 10.0;
    std::string name = "DefaultMat";

    Material() = default;
    TAPI Material(MaterialProp diffuse, MaterialProp ambient, MaterialProp specular, MaterialProp normal, f32 shininess, std::string name) {
        diffuseMat = diffuse;
        ambientMat = ambient;
        specularMat = specular;
        normalMat = normal;
        this->shininess = shininess;
        this->name = name;
    }
    void Delete() {
        diffuseMat.Delete();
        ambientMat.Delete();
        specularMat.Delete();
        normalMat.Delete();
    }
    TAPI void SetShaderUniforms(const Shader& shader, u32 matIdx) const;
};

TAPI void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment = 4);

TAPI u8* LoadImageData(const std::string& imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically = false);

TAPI Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props = TextureProperties::None(), 
                    bool flipVertically = false);
TAPI Texture GenTextureFromImg(u8* imgData, u32 width, u32 height, TextureProperties props);

#endif