//#include "pch.h"
#include "texture.h"
#include "shader.h"
#include "tiny_ogl.h"
#include "tiny_engine.h"
#include "tiny_profiler.h"
#include "tiny_fs.h"
#include "job_system.h"
#include "math/tiny_math.h"


#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include <unordered_map>

struct TextureInternal
{
    u32 oglTexID = 0;
    u32 type = 0;
    u32 width, height = 0;
    std::string texpath = "";
};

class TextureHasher {
public:
 
    size_t operator()(const Texture& p) const
    {
        return p.id;
    }
};

typedef std::unordered_map<Texture, TextureInternal, TextureHasher> TextureCacheMap;

struct TextureCache
{
    // texture path hash -> texture data
    TextureCacheMap cachedTextures = {};
    Texture dummyTexture = Texture();
};

void InitializeTextureCache(Arena* arena)
{
    TextureCache* textureCacheMem = (TextureCache*)arena_alloc(arena, sizeof(TextureCache));
    GetEngineCtx().textureCache = textureCacheMem;
    new(&textureCacheMem->cachedTextures) TextureCacheMap();

    u8 dummyImgData[] = {255,17,250,255};
    Texture dummyTex = LoadGPUTextureFromImg(dummyImgData, 1, 1, TextureProperties::RGBA_NEAREST(), 0xBEEFEE);
    textureCacheMem->dummyTexture = dummyTex;
}

static TextureCache& GetTextureCache()
{
    return *GetEngineCtx().textureCache;
}

Texture GetDummyTexture()
{
    return GetTextureCache().dummyTexture;
}


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
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST_MIPMAP_NEAREST;
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


u32 Texture::OglID() const
{
    TextureCache& texCache = GetTextureCache();
    if (texCache.cachedTextures.count(*this))
    {
        return texCache.cachedTextures[*this].oglTexID;
    }
    return U32_INVALID_ID;
}

void Texture::Delete() 
{
    u32 oglid = OglID();
    if (oglid != U32_INVALID_ID)
    {
        GLCall(glDeleteTextures(1, (const GLuint*)&oglid)); 
    }
    id = U32_INVALID_ID;
}

bool Texture::isValid() const 
{ 
    const TextureInternal& ti = GetTextureCache().cachedTextures[*this];
    return id != U32_INVALID_ID && ti.type != 0; 
}

void Texture::bindUnit(u32 textureUnit) const 
{
    PROFILE_FUNCTION();
    const TextureInternal& ti = GetTextureCache().cachedTextures[*this];
    if (!isValid() || ti.oglTexID == 0)
    {
        // either an invalid texture, or it's still loading
        GetTextureCache().dummyTexture.bindUnit(textureUnit);
    }
    else
    {
        // bind id to given texture unit
        activate(textureUnit);
        u32 oglId = OglID();
        TINY_ASSERT(isValid() && oglId != U32_INVALID_ID && "Attempted to bind texture with invalid type!");
        GLCall(glBindTexture(ti.type, oglId));
    }
}

u32 Texture::GetWidth() const
{
    const TextureInternal& ti = GetTextureCache().cachedTextures[*this];
    return ti.width;
}
u32 Texture::GetHeight() const
{
    const TextureInternal& ti = GetTextureCache().cachedTextures[*this];
    return ti.height;
}
u32 Texture::GetType() const
{
    const TextureInternal& ti = GetTextureCache().cachedTextures[*this];
    return ti.type;
}


void Texture::activate(u32 textureUnit) 
{ 
    GLCall(glActiveTexture(GL_TEXTURE0 + textureUnit)); 
}

// https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glPixelStorei.xhtml
void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment) {
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, offsetX);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, offsetY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


// loads and returns image buffer found at imgPath. 
// NOTE: calling function is responsible for returned buffer lifetime
u8* LoadImageData(const char* imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically) {
    PROFILE_FUNCTION();
    stbi_set_flip_vertically_on_load(shouldFlipVertically);
    u8* data = stbi_load(imgPath, width, height, numChannels, 0);
    if (!data)
    {
        LOG_ERROR("Failed to load image data from %s", imgPath);
        TINY_ASSERT(false);
    }
    return data;
}

u32 GenOpenGLTextureFromImgData(u8* imgData, u32 width, u32 height, TextureProperties props)
{
    u32 ogltexture = U32_INVALID_ID;
    if (imgData) 
    {
        GLCall(glGenTextures(1, &ogltexture));
        GLCall(glBindTexture(GL_TEXTURE_2D, ogltexture));
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
    TINY_ASSERT(ogltexture != U32_INVALID_ID && "Failed to upload image data to gpu");
    return ogltexture;
}

Texture LoadGPUTextureFromImg(u8* imgData, u32 width, u32 height, TextureProperties props, u32 texHash) {
    PROFILE_FUNCTION();
    u32 ogltexture = GenOpenGLTextureFromImgData(imgData, width, height, props);
    Texture ret = Texture(texHash);
    TextureInternal& ti = GetTextureCache().cachedTextures[ret];
    ti.height = height;
    ti.width = width;
    ti.type = GL_TEXTURE_2D;
    ti.oglTexID = ogltexture;
    return ret;
}

Texture Texture::FromGPUTex(u32 oglId, u32 width, u32 height, u32 type)
{
    u32 hash = oglId * width * height + type; // ogl ids should be unique so this might be fine?
    hash = HashBytes((u8*)&hash, sizeof(hash));
    Texture ret = Texture(hash);
    TextureInternal& ti = GetTextureCache().cachedTextures[ret];
    ti.height = height;
    ti.width = width;
    ti.type = type;
    ti.oglTexID = oglId;
    return ret;
}

TextureProperties TexturePropertiesFromImageInfo(u32 numChannels)
{
    TextureProperties props;
    if (numChannels == 3) 
    {
        props = TextureProperties::RGB_LINEAR();
    }
    else if (numChannels == 4) 
    {
        props = TextureProperties::RGBA_LINEAR();
    }
    else if (numChannels == 1) 
    {
        // for L8 images   *UNTESTED*
        props = TextureProperties::RGB_LINEAR();
        props.imgFormat = TextureProperties::ImageFormat::RED;
        props.texFormat = TextureProperties::TexFormat::RED;
        props.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
    }
    else 
    {
        //props = TextureProperties::RGBA_LINEAR();
        TINY_ASSERT(false && "Unknown number of channels in image!");
    }
    return props;
}

// Note: May crash with invalid path/image. Should this return an optional?
Texture LoadTexture(
    const std::string& imgPath, 
    TextureProperties props, 
    bool flipVertically
) {
    PROFILE_FUNCTION();
    u32 strHash = HashBytes((u8*)imgPath.c_str(), imgPath.size());
    Texture tex = Texture(strHash);
    TextureCacheMap& texCache = GetTextureCache().cachedTextures;
    if (texCache.count(tex))
    {
        return tex;
    }
    texCache[tex] = TextureInternal();

    s32 width, height, numChannels = 0;
    u8* data = LoadImageData(imgPath.c_str(), &width, &height, &numChannels, flipVertically);
    // if we didn't specify texture properties, fetch them automatically
    if (props.isNone) 
    {
        props = TexturePropertiesFromImageInfo(numChannels);
    }
    // tex shouldn't change here
    tex = LoadGPUTextureFromImg(data, width, height, props, strHash);
    if (!tex.isValid()) 
    {
        LOG_ERROR("Couldn't load %s", imgPath.c_str());
        TINY_ASSERT(false && "failed to load texture!");
    }
    TextureInternal& ti = GetTextureCache().cachedTextures[tex];
    ti.texpath = imgPath;
    stbi_image_free(data);
    LOG_INFO("Loaded texture %s  channels: %i", imgPath.c_str(), numChannels);
    return tex;
}

Texture LoadTextureAsync(
    const std::string& imgPath, 
    TextureProperties props, 
    TextureLoadSuccessCallback onSuccess,
    bool flipVertically
) {
    PROFILE_FUNCTION();
    u32 strHash = HashBytes((u8*)imgPath.c_str(), imgPath.size());
    Texture tex = Texture(strHash);
    TextureCacheMap& texCache = GetTextureCache().cachedTextures;
    if (texCache.count(tex))
    {
        return tex;
    }
    texCache[tex] = TextureInternal();
    JobSystem::Instance().Execute([strHash, props, imgPath, flipVertically, onSuccess](){
        PROFILE_SCOPE("Load image data");
        s32 width, height, numChannels;
        u8* data = LoadImageData(imgPath.c_str(), &width, &height, &numChannels, flipVertically);     
        JobSystem::Instance().ExecuteOnMainThread([strHash, imgPath, data, width, height, numChannels, props, onSuccess](){
            PROFILE_SCOPE("initialize loaded tex data");
            TextureProperties newProps = props; // cpy to make mutable, lambda captures are const
            if (props.isNone)
            {
                newProps = TexturePropertiesFromImageInfo(numChannels);
            }
            // this also loads in the newly loaded texture id into our global mapping, so the rest of the engine has access to it
            Texture ret = LoadGPUTextureFromImg(data, width, height, newProps, strHash);
            if (!ret.isValid())
            {
                LOG_ERROR("Couldn't load %s", imgPath.c_str());
                TINY_ASSERT(false && "failed to load texture!");
            }
            TextureInternal& ti = GetTextureCache().cachedTextures[ret];
            ti.texpath = imgPath;
            stbi_image_free(data);
            LOG_INFO("Loaded texture %s  channels: %i", imgPath.c_str(), numChannels);
            if (onSuccess)
            {
                onSuccess(ret);
            }
        });
    });
    return tex;
}


void DeleteTexture(Texture tex)
{
    TextureCache& texCache = GetTextureCache();
    TextureInternal& ti = texCache.cachedTextures[tex];
    GLCall(glDeleteTextures(1, &ti.oglTexID)); 
    texCache.cachedTextures.erase(tex);
}
