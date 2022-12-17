#ifndef TINY_FRAMEBUFFER_H
#define TINY_FRAMEBUFFER_H

#include "pch.h"
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
    Framebuffer(){}
    Framebuffer(f32 width, f32 height, FramebufferAttachmentType type);

    inline bool isValid() const { return framebufferID != 0; }
    inline void Bind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID)); 
        GLCall(glViewport(0, 0, size.x, size.y));
    }
    static void BindDefaultFrameBuffer() {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
        GLCall(glViewport(0, 0, (f32)Camera::GetMainCamera().GetScreenWidth(), (f32)Camera::GetMainCamera().GetScreenHeight()));
    }
    void Delete() { 
        GLCall(glDeleteFramebuffers(1, &framebufferID));
        GLCall(glDeleteTextures(1, &texture));
        framebufferID, texture = 0;
    }
    static void ClearDepth() { GLCall(glClear(GL_DEPTH_BUFFER_BIT)); }
    static void ClearStencil() { GLCall(glClear(GL_STENCIL_BUFFER_BIT)); }

    glm::vec2 GetSize() const { return size; }
    Texture GetTexture() const { return Texture(texture); }
    FramebufferAttachmentType type = COLOR;
private:
    u32 framebufferID = 0;
    u32 texture = 0;
    glm::vec2 size = glm::vec2(0);
};

struct ShadowMap {
    ShadowMap(){}
    ShadowMap(u32 resolution);
    bool isValid() const { return fb.isValid(); }
    void BeginRender() const;
    void RenderToShadowMap(const Light& light, Model& model, const Transform& tf, s32 depthTexTextureUnit) const;
    void EndRender() const;
    void SetShadowUniforms(const Shader& shader, const Light& light, s32 depthTexTextureUnit) const;

    Framebuffer fb;
    Shader depthShader;
};

#endif