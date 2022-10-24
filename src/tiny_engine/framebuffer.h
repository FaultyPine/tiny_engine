#ifndef TINY_RENDER_TEXTURE_H
#define TINY_RENDER_TEXTURE_H

#include "pch.h"
#include "sprite.h"
#include "tiny_engine.h"

// if not using a fullscreen buffer, make sure to set glViewport to prevent stretching/weird stuff happening with the rendered texture

struct FullscreenFrameBuffer {
    FullscreenFrameBuffer(glm::vec2 size);
    FullscreenFrameBuffer() {}

    inline bool isValid() { return framebufferID != 0; }
    inline void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, framebufferID); }
    inline void BindDefaultFrameBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    void Delete() { 
        glDeleteFramebuffers(1, &framebufferID);
        glDeleteTextures(1, &textureColorBufferID);
        glDeleteRenderbuffers(1, &renderBufferObjectID);
    }
    void ClearColor() { 
        ClearGLColorBuffer();
    }
    void ClearDepth() { glClear(GL_DEPTH_BUFFER_BIT); }
    void ClearStencil() { glClear(GL_STENCIL_BUFFER_BIT); }
    void DrawToScreen(const Shader& shader) {
        fullscreenSprite.OverrideSpriteShader(shader);
        BindDefaultFrameBuffer();
        ClearColor();
        shader.use();
        shader.setUniform("screenWidth", Camera::GetScreenWidth());
        shader.setUniform("screenHeight", Camera::GetScreenHeight());
        shader.setUniform("time", (f32)GetTime());
        fullscreenSprite.DrawSprite(
            Camera::GetMainCamera(), 
            {0,0}, size);
    }

private:
    glm::vec2 size = {0.0, 0.0};
    Sprite fullscreenSprite;
    u32 framebufferID = 0;
    u32 textureColorBufferID = 0;
    u32 renderBufferObjectID = 0;

};


#endif