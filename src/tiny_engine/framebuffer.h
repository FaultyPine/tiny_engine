#ifndef TINY_RENDER_TEXTURE_H
#define TINY_RENDER_TEXTURE_H

#include "pch.h"

struct FrameBuffer {
    FrameBuffer();

    inline void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, framebufferID); }
    inline void BindDefaultFrameBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

private:
    u32 framebufferID = 0;
    u32 textureColorBufferID = 0;
    u32 renderBufferObjectID = 0;

};


#endif