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
    Framebuffer() = default;
    TAPI Framebuffer(u32 width, u32 height, u32 numColorAttachments = 1, bool depthAttachment = false);

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

    TAPI glm::vec2 GetSize() const { return size; }
    TAPI Texture GetColorTexture(u32 i) const { return colorTextures[i]; }
    TAPI Texture GetDepthTexture() const { return depthTex; }
    TAPI void AttachTexture(FramebufferAttachmentType type, const Texture& tex);

    Texture depthTex = {};
    Texture colorTextures[TextureMaterialType::NUM_MATERIAL_TYPES] = {};
    u32 framebufferID = U32_INVALID_ID;
    u32 renderBufferObjectID = 0;
    glm::vec2 size = glm::vec2(0);
    Sprite visualizationSprite = {};
};

#endif