#ifndef TINY_TEXTURE_H
#define TINY_TEXTURE_H

#include "tiny_defines.h"
#include <string>
#include <functional>
#include "tiny_ogl.h"

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
    RGB16F = GL_RGB16F,
    RGB32F = GL_RGB32F,
    RGBA = GL_RGBA,
    RGBA16F = GL_RGBA16F,
    RGBA32F = GL_RGBA32F,
    RG = GL_RG,
    RED = GL_RED,
    DEPTH_STENCIL = GL_DEPTH_STENCIL,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
};
enum class ImageFormat : s32 {
    RGB = GL_RGB,
    RGB16F = GL_RGB16F,
    RGB32F = GL_RGB32F,
    RGBA = GL_RGBA,
    RGBA16F = GL_RGBA16F,
    RGBA32F = GL_RGBA32F,
    RED = GL_RED,
};
enum class ImageDataType : s32 {
    BYTE = GL_BYTE,
    UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    SHORT = GL_SHORT,
    UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
    INT = GL_INT,
    UNSIGNED_INT = GL_UNSIGNED_INT,
    FLOAT = GL_FLOAT,
};

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
    // handle to a TextureInternal owned by the TextureCache
    u32 id = U32_INVALID_ID; 

    Texture() = default;
    explicit Texture(u32 id) { this->id = id; }
    bool isValid() const;
    void Delete();
    TAPI u32 OglID() const;
    u32 GetWidth() const;
    u32 GetHeight() const;
    u32 GetType() const;
    void bindUnit(u32 textureUnit) const;
    static Texture FromGPUTex(u32 oglId, u32 width, u32 height, u32 type);
    static void activate(u32 textureUnit);

    bool operator==(const Texture& p) const
    {
        return id == p.id;
    }
};

typedef std::function<void(Texture onSuccessTexture)> TextureLoadSuccessCallback;

struct Arena;
void InitializeTextureCache(Arena* arena);

TAPI void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment = 4);

TAPI u8* LoadImageData(const char* imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically = false);

TAPI Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props = TextureProperties::None(), 
                    bool flipVertically = false);
TAPI Texture LoadTextureAsync(
    const std::string& imgPath, 
    TextureProperties props = TextureProperties::None(),
    TextureLoadSuccessCallback onSuccess = {},
    bool flipVertically = false);
Texture LoadGPUTextureFromImg(u8* imgData, u32 width, u32 height, TextureProperties props, u32 texHash);

Texture GetDummyTexture();

void DeleteTexture(Texture tex);

#endif