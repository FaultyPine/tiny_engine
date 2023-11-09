#ifndef TINY_FRAMEBUFFER_H
#define TINY_FRAMEBUFFER_H

//#include "pch.h"
#include "texture.h"
#include "camera.h"
#include "tiny_types.h"
#include "shader.h"

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

    TAPI glm::vec2 GetSize() const { return size; }
    TAPI Texture GetTexture() const { return texture; }
    FramebufferAttachmentType type = COLOR;

    u32 framebufferID = 0;
    u32 renderBufferObjectID = 0;
    Texture texture = {};
    glm::vec2 size = glm::vec2(0);
};

struct LightDirectional;
struct Model;
struct ShadowMap {
    ShadowMap() = default;
    TAPI ShadowMap(u32 resolution);
    void Delete() {
        fb.Delete();
        depthShader.Delete();
    }
    bool isValid() const { return fb.isValid(); }
    TAPI void BeginRender() const;
    // makes the passed in shader "receive" shadows (just gets shadow uniforms set)
    TAPI void ReceiveShadows(Shader& shader, const LightDirectional& light) const;
    // makes passed in model cast shadows (just renders the model into the depth texture)
    TAPI void RenderShadowCaster(const LightDirectional& light, Model& model, const Transform& tf) const;
    TAPI void EndRender() const;

    Framebuffer fb;
    Shader depthShader;
};

TAPI Framebuffer CreateDepthAndNormalsFB(f32 width, f32 height);

#endif