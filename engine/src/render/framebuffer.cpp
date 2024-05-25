//#include "pch.h"
#include "framebuffer.h"
#include "tiny_engine.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "shader.h"
#include "render/model.h"
#include "tiny_profiler.h"


static void GenerateTexturesForFramebuffer(
    glm::vec2 size, 
    Texture* colorTextures, 
    u32 numColorTextures, 
    Texture* depthTexture,
    Framebuffer::FramebufferProperties::AttachmentProperties* attachmentProperties)
{
    TINY_ASSERT(numColorTextures <= FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS);
    for (u32 i = 0; i < numColorTextures; i++)
    {
        u32 colorTextureID = 0;
        glGenTextures(1, &colorTextureID);
        glBindTexture(GL_TEXTURE_2D, colorTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
        Framebuffer::FramebufferProperties::AttachmentProperties attachmentProps = attachmentProperties[i];
        // generate texture to attach to framebuffer (alloc vid mem)
        //           something like               GL_RGBA16F                                           GL_RGBA             GL_FLOAT
        glTexImage2D(GL_TEXTURE_2D, 0, attachmentProps.internalFormat, size.x, size.y, 0, attachmentProps.format, attachmentProps.dataType, NULL);
        // attach texture to framebuffer
        s32 oglFramebufferAttachment = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, oglFramebufferAttachment, GL_TEXTURE_2D, colorTextureID, 0);
        Texture colorTexture = Texture::FromGPUTex(colorTextureID, size.x, size.y, GL_TEXTURE_2D);
        colorTextures[i] = colorTexture;
    }

    if (depthTexture != nullptr)
    {
        u32 depthTexID = 0;
        glGenTextures(1, &depthTexID);
        glBindTexture(GL_TEXTURE_2D, depthTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // if its a depth texture, everything outside our texture should default to 1
        // I.E. if there is a pixel in the normal camera that isn't covered by the shadow map, give it depth=1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr f32 borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
        // generate texture to attach to framebuffer (alloc vid mem)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        // disable color buffer if it's a depth tex
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexID, 0);
        Texture tex = Texture::FromGPUTex(depthTexID, size.x, size.y, GL_TEXTURE_2D);
        *depthTexture = tex;
    }
}

Framebuffer::Framebuffer(u32 width, u32 height, u32 numColorAttachments, bool depthAttachment)
{
    FramebufferProperties props = {};
    props.size = glm::vec2(width, height);
    props.numColorAttachments = numColorAttachments;
    props.hasDepth = depthAttachment;
    *this = Framebuffer(&props);
}

Framebuffer::Framebuffer(FramebufferProperties* properties)
{
    PROFILE_FUNCTION();
    this->properties = *properties;
    // generate and bind a framebuffer object
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);    
    Texture newColorTextures[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
    Texture newDepthTexture = {};
    GenerateTexturesForFramebuffer(
        properties->size, 
        newColorTextures, 
        properties->numColorAttachments, 
        properties->hasDepth ? &newDepthTexture : nullptr,
        properties->colorAttachments);
    if (!properties->hasDepth)
    {
        // to make sure opengl can do depth (or stencil) testing, gotta add a depth/stencil attachment
        glGenRenderbuffers(1, &renderBufferObjectID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObjectID); 
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, properties->size.x, properties->size.y);  
        // attach the renderbuffer object to the depth and stencil attachment of the framebuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObjectID);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    TINY_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    TMEMCPY(this->colorTextures, newColorTextures, sizeof(Texture) * ARRAY_SIZE(this->colorTextures));
    this->depthTex = newDepthTexture;
    visualizationSprite = Sprite(GetColorTexture(0));
}

bool Framebuffer::isValid() const { return framebufferID != U32_INVALID_ID; }
void Framebuffer::Bind() const {
    PROFILE_FUNCTION();
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID); 
    GLuint attachments[FramebufferAttachmentType::MAX_NUM_COLOR_ATTACHMENTS] = {};
    u32 numAttachments = 0;
    // bind all color attachments. I don't think we'll ever be writing to one individual attachment and not the others
    for (u32 i = 0; i < ARRAY_SIZE(colorTextures); i++)
    {
        if (colorTextures[i].isValid())
        {
            attachments[numAttachments] = GL_COLOR_ATTACHMENT0+i;
            numAttachments++;
        }
    }
    // extra attachments need to be specified before drawing
    glDrawBuffers(numAttachments, attachments);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferID);
    glViewport(0, 0, properties.size.x, properties.size.y);
}
void Framebuffer::BindDefaultFrameBuffer() {
    PROFILE_FUNCTION();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glViewport(0, 0, (f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight());
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

void Framebuffer::Blit(const Framebuffer* src, const Framebuffer* dst, bool isDepth)
{
    if (dst->properties.size.x < src->properties.size.x || dst->properties.size.y < src->properties.size.y)
    {
        return;
    }
    Framebuffer::Blit(
        src->framebufferID, 
        0, 0,
        src->properties.size.x, src->properties.size.y,
        dst->framebufferID,
        0, 0,
        dst->properties.size.x, dst->properties.size.y, isDepth);
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
    glDeleteFramebuffers(1, &framebufferID);
    for (u32 i = 0; i < ARRAY_SIZE(colorTextures); i++)
    {
        colorTextures[i].Delete();
    }
    glDeleteRenderbuffers(1, &renderBufferObjectID);
    framebufferID = 0; 
    TMEMSET(colorTextures, 0, sizeof(Texture)*ARRAY_SIZE(colorTextures)); 
    depthTex.Delete();
    renderBufferObjectID = 0;
    properties = {};
}
void Framebuffer::ClearDepth() { glClear(GL_DEPTH_BUFFER_BIT); }
void Framebuffer::ClearStencil() { glClear(GL_STENCIL_BUFFER_BIT); }
