#ifndef TINY_FRAMEBUFFER_H
#define TINY_FRAMEBUFFER_H

//#include "pch.h"
#include "texture.h"
#include "camera.h"
#include "tiny_types.h"
#include "render/sprite.h"

struct Framebuffer {
    enum FramebufferAttachmentType {
        COLOR,
        DEPTH,
    };
    Framebuffer() = default;
    TAPI Framebuffer(f32 width, f32 height, FramebufferAttachmentType type = FramebufferAttachmentType::COLOR);

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
                    FramebufferAttachmentType type);
    TAPI void DrawToFramebuffer(const Framebuffer& dstFramebuffer, const Transform2D& dst) const;

    TAPI glm::vec2 GetSize() const { return size; }
    TAPI Texture GetTexture() const { return texture; }

    FramebufferAttachmentType type = COLOR;
    u32 framebufferID = 0xDEADBEEF;
    u32 renderBufferObjectID = 0;
    Texture texture = {};
    glm::vec2 size = glm::vec2(0);
    Sprite visualizationSprite;
};


TAPI Framebuffer CreateDepthAndNormalsFB(f32 width, f32 height);

#endif