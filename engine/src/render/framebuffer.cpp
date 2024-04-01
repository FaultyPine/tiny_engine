//#include "pch.h"
#include "framebuffer.h"
#include "tiny_engine.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "shader.h"
#include "render/model.h"


// colortextures is filled out based on the numColorTextures. depthTexture is nullptr for no depth or points to a u32 to be filled out
static void GenerateTexturesForFramebuffer(glm::vec2 size, Texture* colorTextures, u32 numColorTextures, Texture* depthTexture)
{
    TINY_ASSERT(numColorTextures <= FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS);
    for (u32 i = 0; i < numColorTextures; i++)
    {
        u32 colorTextureID = 0;
        GLCall(glGenTextures(1, &colorTextureID));
        GLCall(glBindTexture(GL_TEXTURE_2D, colorTextureID));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); 
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); 
        s32 component = GL_RGB;
        s32 dataType = GL_UNSIGNED_BYTE;
        // generate texture to attach to framebuffer (alloc vid mem)
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, component, size.x, size.y, 0, component, dataType, NULL));
        // attach texture to framebuffer
        s32 oglFramebufferAttachment = GL_COLOR_ATTACHMENT0 + i;
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, oglFramebufferAttachment, GL_TEXTURE_2D, colorTextureID, 0));
        Texture colorTexture = Texture::FromGPUTex(colorTextureID, size.x, size.y, GL_TEXTURE_2D);
        colorTextures[i] = colorTexture;
    }

    if (depthTexture != nullptr)
    {
        u32 depthTexID = 0;
        GLCall(glGenTextures(1, &depthTexID));
        GLCall(glBindTexture(GL_TEXTURE_2D, depthTexID));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        // if its a depth texture, everything outside our texture should default to 1
        // I.E. if there is a pixel in the normal camera that isn't covered by the shadow map, give it depth=1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr f32 borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
        // generate texture to attach to framebuffer (alloc vid mem)
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
        // disable color buffer if it's a depth tex
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // attach texture to framebuffer
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexID, 0));
        Texture tex = Texture::FromGPUTex(depthTexID, size.x, size.y, GL_TEXTURE_2D);
        *depthTexture = tex;
    }
}

Framebuffer::Framebuffer(u32 width, u32 height, u32 numColorAttachments, bool depthAttachment)
{
    this->size = glm::vec2(width, height);
    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));    
    Texture newColorTextures[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
    Texture newDepthTexture = {};
    GenerateTexturesForFramebuffer(this->size, newColorTextures, numColorAttachments, depthAttachment ? &newDepthTexture : nullptr);
    if (numColorAttachments > 0)
    {
        // to make sure opengl can do depth (or stencil) testing, gotta add a depth/stencil attachment
        GLCall(glGenRenderbuffers(1, &renderBufferObjectID));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObjectID)); 
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y));  
        // attach the renderbuffer object to the depth and stencil attachment of the framebuffer
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObjectID));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer is not complete!");
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));  

    TMEMCPY(this->colorTextures, newColorTextures, sizeof(Texture) * ARRAY_SIZE(this->colorTextures));
    this->depthTex = newDepthTexture;
    visualizationSprite = Sprite(GetColorTexture(0));
}

bool Framebuffer::isValid() const { return framebufferID != U32_INVALID_ID; }
void Framebuffer::Bind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID)); 
    GLuint attachments[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
    u32 numAttachments = 0;
    for (u32 i = 0; i < ARRAY_SIZE(colorTextures); i++)
    {
        if (colorTextures[i].isValid())
        {
            u32 aoTexID = colorTextures[i].OglID();
            attachments[numAttachments] = GL_COLOR_ATTACHMENT0+i;
            numAttachments++;
        }
    }
    // extra attachments need manual "activation" before drawing.
    glDrawBuffers(numAttachments, attachments);
    GLCall(glViewport(0, 0, size.x, size.y));
}
void Framebuffer::BindDefaultFrameBuffer() {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
    GLCall(glViewport(0, 0, (f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight()));
}
void Framebuffer::Blit(
    u32 framebufferSrc,
    f32 srcX0, f32 srcY0,
    f32 srcX1, f32 srcY1,
    u32 framebufferDst, 
    f32 dstX0, f32 dstY0, 
    f32 dstX1, f32 dstY1,
    bool isDepth)
{
    u32 glFbType = GL_COLOR_BUFFER_BIT;
    if (isDepth)
    {
        glFbType = GL_DEPTH_BUFFER_BIT;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferSrc);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferDst);
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, glFbType, GL_NEAREST);
}

void Framebuffer::DrawToFramebuffer(
    const Framebuffer& dstFramebuffer, 
    const Transform2D& dst,
    FramebufferAttachmentType attachment) const
{
    dstFramebuffer.Bind();
    Texture texToDraw;
    if (attachment < FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS)
    {
        texToDraw = colorTextures[attachment];
    }
    else if (attachment == FramebufferAttachmentType::DEPTH)
    {
        texToDraw = depthTex;
    }
    visualizationSprite.DrawSprite(texToDraw, dst.position, dst.scale, dst.rotation, {0,0,1}, glm::vec4(1), true);
}

void Framebuffer::Delete() { 
    GLCall(glDeleteFramebuffers(1, &framebufferID));
    for (u32 i = 0; i < ARRAY_SIZE(colorTextures); i++)
    {
        colorTextures[i].Delete();
    }
    GLCall(glDeleteRenderbuffers(1, &renderBufferObjectID));
    framebufferID = 0; 
    TMEMSET(colorTextures, 0, sizeof(Texture)*ARRAY_SIZE(colorTextures)); 
    depthTex.Delete();
    renderBufferObjectID = 0;
    size = glm::vec2(0);
}
void Framebuffer::ClearDepth() { GLCall(glClear(GL_DEPTH_BUFFER_BIT)); }
void Framebuffer::ClearStencil() { GLCall(glClear(GL_STENCIL_BUFFER_BIT)); }


Framebuffer CreateDepthAndNormalsFB(f32 width, f32 height) {
    Framebuffer fb;
    fb.size = glm::vec2(width, height);
    u32 textureID = -1;

    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &fb.framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fb.framebufferID));

    // generate our texture that we'll draw to
    GLCall(glGenTextures(1, &textureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE));

    // using RGBA32F to ensure our depth values don't lose precision when going from the depth buffer
    // to our texture. (normally textures might store the values with less precision than the depth buffer would)
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0));

    // when rendering to this framebuffer, opengl needs a place to store depth values.
    // it won't store them in the texture so this renderbuffer gives it a place to do depth stuff
    // using a renderbuffer instead of a depth texture attachment since we won't need to sample these depth values
    // since they're stored in the color texture attachment anyway and if you don't need to sample from it, renderbuffers are
    // faster than textures (or so im told)
    GLCall(glGenRenderbuffers(1, &fb.renderBufferObjectID));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, fb.renderBufferObjectID));
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
    // attach the renderbuffer object to the depth and stencil attachment of the framebuffer
    GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.renderBufferObjectID));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer is not complete!");
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
 
    Texture result = Texture::FromGPUTex(textureID, width, height, GL_TEXTURE_2D);
    fb.colorTextures[0] = result;
    fb.visualizationSprite = Sprite(fb.GetColorTexture(0));
    return fb;
}