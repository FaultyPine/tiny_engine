//#include "pch.h"
#include "framebuffer.h"
#include "tiny_engine.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "shader.h"
#include "render/model.h"
#include "tiny_profiler.h"


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
        // generate texture to attach to framebuffer (alloc vid mem)
        // TODO: make configurable
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL));
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
    PROFILE_FUNCTION();
    this->size = glm::vec2(width, height);
    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));    
    Texture newColorTextures[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
    Texture newDepthTexture = {};
    GenerateTexturesForFramebuffer(this->size, newColorTextures, numColorAttachments, depthAttachment ? &newDepthTexture : nullptr);
    if (!depthAttachment)
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
    PROFILE_FUNCTION();
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
    PROFILE_FUNCTION();
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
    PROFILE_FUNCTION();
    u32 glFbType = GL_COLOR_BUFFER_BIT;
    if (isDepth)
    {
        glFbType = GL_DEPTH_BUFFER_BIT;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferSrc);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferDst);
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, glFbType, GL_NEAREST);
}

void Framebuffer::Blit(Framebuffer* src, Framebuffer* dst)
{
    if (dst->size.x < src->size.x || dst->size.y < src->size.y)
    {
        return;
    }
    Framebuffer::Blit(
        src->framebufferID, 
        0, 0,
        src->size.x, src->size.y,
        dst->framebufferID,
        0, 0,
        dst->size.x, dst->size.y);
}

void Framebuffer::AttachTexture(FramebufferAttachmentType type, const Texture& tex)
{
    if (type < FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS)
    {
        if (colorTextures[type].isValid())
        {
            LOG_WARN("Attempted to attach texture to framebuffer with already existing texture at this location!");
        }
        else
        {
            colorTextures[type] = tex;
        }
    }
    else
    {
        depthTex = tex;
    }
}

void Framebuffer::DrawToFramebuffer(
    const Framebuffer& dstFramebuffer, 
    const Transform2D& dst,
    FramebufferAttachmentType attachment,
    const Shader& shader) const
{
    PROFILE_FUNCTION();
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
    if (shader.isValid())
    {
        visualizationSprite.DrawSprite(shader, texToDraw, dst.position, dst.scale, dst.rotation, {0,0,1}, glm::vec4(1), true);
    }
    else
    {
        visualizationSprite.DrawSprite(texToDraw, dst.position, dst.scale, dst.rotation, {0,0,1}, glm::vec4(1), true);
    }
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
