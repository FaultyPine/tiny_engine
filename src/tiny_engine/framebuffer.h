#ifndef TINY_FRAMEBUFFER_H
#define TINY_FRAMEBUFFER_H

//#include "pch.h"
#include "texture.h"
#include "camera.h"
#include "model.h"
#include "tiny_types.h"
// possible TODO: give easy api func that draws this framebuffer to the screen

struct Framebuffer {
    enum FramebufferAttachmentType {
        COLOR = GL_COLOR_ATTACHMENT0,
        DEPTH = GL_DEPTH_ATTACHMENT,
    };
    Framebuffer() = default;
    Framebuffer(f32 width, f32 height, FramebufferAttachmentType type = FramebufferAttachmentType::COLOR);

    inline bool isValid() const { return framebufferID != 0; }
    inline void Bind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID)); 
        GLCall(glViewport(0, 0, size.x, size.y));
    }
    static void BindDefaultFrameBuffer() {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
        GLCall(glViewport(0, 0, (f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight()));
    }
    void Delete() { 
        GLCall(glDeleteFramebuffers(1, &framebufferID));
        GLCall(glDeleteTextures(1, &texture));
        GLCall(glDeleteRenderbuffers(1, &renderBufferObjectID));
        framebufferID = 0; texture = 0; renderBufferObjectID = 0;
        size = glm::vec2(0);
    }
    static void ClearDepth() { GLCall(glClear(GL_DEPTH_BUFFER_BIT)); }
    static void ClearStencil() { GLCall(glClear(GL_STENCIL_BUFFER_BIT)); }

    glm::vec2 GetSize() const { return size; }
    Texture GetTexture() const { return Texture(texture); }
    FramebufferAttachmentType type = COLOR;

    u32 framebufferID = 0;
    u32 renderBufferObjectID = 0;
    u32 texture = 0;
    glm::vec2 size = glm::vec2(0);
};

struct ShadowMap {
    ShadowMap() = default;
    ShadowMap(u32 resolution);
    void Delete() {
        fb.Delete();
        depthShader.Delete();
    }
    bool isValid() const { return fb.isValid(); }
    void BeginRender() const;
    // makes the passed in shader "receive" shadows (just gets shadow uniforms set)
    void ReceiveShadows(Shader& shader, const Light& light) const;
    // makes passed in model cast shadows (just renders the model into the depth texture)
    void RenderShadowCaster(const Light& light, Model& model, const Transform& tf) const;
    void EndRender() const;

    Framebuffer fb;
    Shader depthShader;
};

Framebuffer CreateDepthAndNormalsFB(f32 width, f32 height);

#endif