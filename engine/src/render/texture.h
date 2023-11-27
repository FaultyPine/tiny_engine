#ifndef TINY_TEXTURE_H
#define TINY_TEXTURE_H

#include "tiny_defines.h"
#include <string>
#include <functional>

enum TextureMaterialType {
    DIFFUSE = 0,
    AMBIENT,
    SPECULAR,
    NORMAL,
    ROUGHNESS,
    EMISSION,
    OTHER,

    NUM_MATERIAL_TYPES,
};

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
    u32 id = U32_INVALID_ID; // this is the actual opengl texture id, the rest of these fields are just extra info for convinience

    Texture() = default;
    Texture(u32 id) { this->id = id; }
    bool isValid() const;
    void Delete();
    u32 OglID() const;
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

void DeleteTexture(Texture tex);

#endif