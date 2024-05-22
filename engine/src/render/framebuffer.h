#ifndef TINY_FRAMEBUFFER_H
#define TINY_FRAMEBUFFER_H

//#include "pch.h"
#include "texture.h"
#include "camera.h"
#include "tiny_types.h"
#include "render/sprite.h"
#include "tiny_material.h"

enum FramebufferAttachmentType : u32 
{
    COLOR0 = 0,
    COLOR1,
    COLOR2,
    COLOR3,
    COLOR4,
    COLOR5,
    COLOR6,
    COLOR7, // spec guaranteed

    MAX_NUM_COLOR_ATTACHMENTS,
    DEPTH,
};



struct Framebuffer 
{
    struct FramebufferProperties
    {
        enum FramebufferInternalFormat
        {
            R8 = GL_R8,R8_SNORM = GL_R8_SNORM,R16 = GL_R16,R16_SNORM = GL_R16_SNORM,RG8 = GL_RG8,RG8_SNORM = GL_RG8_SNORM,RG16 = GL_RG16,RG16_SNORM = GL_RG16_SNORM,R3_G3_B2 = GL_R3_G3_B2,RGB4 = GL_RGB4,RGB5 = GL_RGB5,RGB8 = GL_RGB8,RGB8_SNORM = GL_RGB8_SNORM,RGB10 = GL_RGB10,RGB12 = GL_RGB12,RGB16_SNORM = GL_RGB16_SNORM,RGBA2 = GL_RGBA2,RGBA4 = GL_RGBA4,RGB5_A1 = GL_RGB5_A1,RGBA8 = GL_RGBA8,RGBA8_SNORM = GL_RGBA8_SNORM,RGB10_A2 = GL_RGB10_A2,RGB10_A2UI = GL_RGB10_A2UI,RGBA12 = GL_RGBA12,RGBA16 = GL_RGBA16,SRGB8 = GL_SRGB8,SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,R16F = GL_R16F,RG16F = GL_RG16F,RGB16F = GL_RGB16F,RGBA16F = GL_RGBA16F,R32F = GL_R32F,RG32F = GL_RG32F,RGB32F = GL_RGB32F,RGBA32F = GL_RGBA32F,R11F_G11F_B10F = GL_R11F_G11F_B10F,RGB9_E5 = GL_RGB9_E5,R8I = GL_R8I,R8UI = GL_R8UI,R16I = GL_R16I,R16UI = GL_R16UI,R32I = GL_R32I,R32UI = GL_R32UI,RG8I = GL_RG8I,RG8UI = GL_RG8UI,RG16I = GL_RG16I,RG16UI = GL_RG16UI,RG32I = GL_RG32I,RG32UI = GL_RG32UI,RGB8I = GL_RGB8I,RGB8UI = GL_RGB8UI,RGB16I = GL_RGB16I,RGB16UI = GL_RGB16UI,RGB32I = GL_RGB32I,RGB32UI = GL_RGB32UI,RGBA8I = GL_RGBA8I,RGBA8UI = GL_RGBA8UI,RGBA16I = GL_RGBA16I,RGBA16UI = GL_RGBA16UI,RGBA32I = GL_RGBA32I,RGBA32UI = GL_RGBA32UI,
        };
        enum FramebufferFormat
        {
            RED = GL_RED,RG = GL_RG,RGB = GL_RGB,BGR = GL_BGR,RGBA = GL_RGBA,BGRA = GL_BGRA,RED_INTEGER = GL_RED_INTEGER,RG_INTEGER = GL_RG_INTEGER,RGB_INTEGER = GL_RGB_INTEGER,BGR_INTEGER = GL_BGR_INTEGER,RGBA_INTEGER = GL_RGBA_INTEGER,BGRA_INTEGER = GL_BGRA_INTEGER,STENCIL_INDEX = GL_STENCIL_INDEX,DEPTH_COMPONENT = GL_DEPTH_COMPONENT,DEPTH_STENCIL,   
        };
        enum PixelDataType
        {
            UNSIGNED_BYTE = GL_UNSIGNED_BYTE, BYTE = GL_BYTE, UNSIGNED_SHORT = GL_UNSIGNED_SHORT, SHORT = GL_SHORT, UNSIGNED_INT = GL_UNSIGNED_INT, INT = GL_INT, HALF_FLOAT = GL_HALF_FLOAT, FLOAT = GL_FLOAT,
        };
        struct AttachmentProperties
        {
            FramebufferInternalFormat internalFormat = FramebufferInternalFormat::RGBA16F;
            FramebufferFormat format = FramebufferFormat::RGBA;
            PixelDataType dataType = PixelDataType::FLOAT;
            const char* attachmentName = "Unnamed Color Attachment";
        };
        glm::u16vec2 size = glm::u16vec2(0); // graphics drivers allow differing size attachments, simplifying it here - all attachments must be same size
        AttachmentProperties colorAttachments[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
        u32 numColorAttachments = 1;
        bool hasDepth = false;
    };
    Framebuffer() = default;
    TAPI Framebuffer(
        u32 width, 
        u32 height, 
        u32 numColorAttachments = 1, 
        bool depthAttachment = false);
    TAPI Framebuffer(FramebufferProperties* properties);

    TAPI bool isValid() const;
    TAPI void Bind() const;
    TAPI static void BindDefaultFrameBuffer();
    TAPI void Delete();
    TAPI static void ClearDepth();
    TAPI static void ClearStencil();
    TAPI static void Blit(
                    u32 framebufferSrc,
                    f32 srcX0, f32 srcY0,
                    f32 srcX1, f32 srcY1,
                    u32 framebufferDst, 
                    f32 dstX0, f32 dstY0, 
                    f32 dstX1, f32 dstY1,
                    bool isDepth = false);
    // blit to (0,0) (ensures dst is bigger than src)
    TAPI static void Blit(Framebuffer* src, Framebuffer* dst);
    TAPI void DrawToFramebuffer(
        const Framebuffer& dstFramebuffer, 
        const Transform2D& dst, 
        FramebufferAttachmentType attachment,
        const Shader& shader) const;
    void DrawToFramebuffer(
        const Framebuffer& dstFramebuffer, 
        FramebufferAttachmentType attachment,
        const Shader& shader) const
        { DrawToFramebuffer(dstFramebuffer, Transform2D({0,0}, dstFramebuffer.GetSize()), attachment, shader); }

    TAPI glm::vec2 GetSize() const { return properties.size; }
    TAPI Texture GetColorTexture(u32 i) const { return colorTextures[i]; }
    TAPI Texture GetDepthTexture() const { return depthTex; }
    TAPI void AttachTexture(FramebufferAttachmentType type, const Texture& tex);

    bool operator==(const Framebuffer& fb) const { return framebufferID == fb.framebufferID; }

    Texture depthTex = {};
    Texture colorTextures[TextureMaterialType::NUM_MATERIAL_TYPES] = {};
    u32 framebufferID = U32_INVALID_ID;
    u32 renderBufferObjectID = 0;
    Sprite visualizationSprite = {};
    FramebufferProperties properties = {};
};

#endif