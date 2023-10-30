//#include "pch.h"
#include "framebuffer.h"
#include "tiny_engine.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "shader.h"

static s32 GetOGLFramebufferAttachmentType(Framebuffer::FramebufferAttachmentType type)
{
    s32 oglFramebufferType;
    if (type == Framebuffer::FramebufferAttachmentType::DEPTH)
    {
        return GL_DEPTH_ATTACHMENT;
    }
    else if (type == Framebuffer::FramebufferAttachmentType::COLOR)
    {
        return GL_COLOR_ATTACHMENT0;
    }
    LOG_WARN("Passed unaccounted-for framebuffer attachment type to GetOGLFramebufferAttachmentType");
    return GL_COLOR_ATTACHMENT0;
}

Framebuffer::Framebuffer(f32 width, f32 height, FramebufferAttachmentType fbtype) {
    this->type = fbtype;
    this->size = glm::vec2(width, height);

    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));    

    // generate our texture that we'll draw to
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));

    // texture setup
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    if (type == DEPTH) {
        // if its a depth texture, everything outside our texture should default to 1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr f32 borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    }
    else {
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); 
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); 
    }
    s32 component = type == DEPTH ? GL_DEPTH_COMPONENT : GL_RGB;
    s32 dataType = type == DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE;
    // generate texture to attach to framebuffer
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, component, width, height, 0, component, dataType, NULL));
    if (type == DEPTH) {
        // disable color buffer if it's a depth tex
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    // attach texture to framebuffer
    s32 oglFramebufferAttachment = GetOGLFramebufferAttachmentType(type);
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, oglFramebufferAttachment, GL_TEXTURE_2D, texture, 0));

    if (type == COLOR) {
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
}

bool Framebuffer::isValid() const { return framebufferID != 0; }
void Framebuffer::Bind() const {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID)); 
    GLCall(glViewport(0, 0, size.x, size.y));
}
void Framebuffer::BindDefaultFrameBuffer() {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
    GLCall(glViewport(0, 0, (f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight()));
}
void Framebuffer::Delete() { 
    GLCall(glDeleteFramebuffers(1, &framebufferID));
    GLCall(glDeleteTextures(1, &texture));
    GLCall(glDeleteRenderbuffers(1, &renderBufferObjectID));
    framebufferID = 0; texture = 0; renderBufferObjectID = 0;
    size = glm::vec2(0);
}
void Framebuffer::ClearDepth() { GLCall(glClear(GL_DEPTH_BUFFER_BIT)); }
void Framebuffer::ClearStencil() { GLCall(glClear(GL_STENCIL_BUFFER_BIT)); }


ShadowMap::ShadowMap(u32 resolution) {
    fb = Framebuffer(resolution, resolution, Framebuffer::FramebufferAttachmentType::DEPTH);
    // this shader renders our scene from the perspective of a light
    depthShader = Shader(ResPath("shaders/depth.vert"), ResPath("shaders/depth.frag"));
}
void ShadowMap::BeginRender() const {
    fb.Bind();
    // completely clear depth texture
    ClearGLBuffers();
    // cull front faces when rendering to depth tex
    glCullFace(GL_FRONT);
}
void ShadowMap::EndRender() const {
    // reset cull mode
    glCullFace(GL_BACK);
    // bind default fb
    Framebuffer::BindDefaultFrameBuffer();
}
void ShadowMap::ReceiveShadows(Shader& shader, const Light& light) const {
    if (!shader.isValid()) return;
    shader.use();
    shader.TryAddSampler(fb.GetTexture().id, "shadowMap");
    shader.setUniform("lightSpaceMatrix", light.GetLightViewProjMatrix());
}
void ShadowMap::RenderShadowCaster(const Light& light, Model& model, const Transform& tf) const {
    if (!depthShader.isValid()) return;
    depthShader.use();
    glm::mat4 lightMat = light.GetLightViewProjMatrix();
    glm::mat4 modelMat = tf.ToModelMatrix();
    glm::mat4 mvp = lightMat * modelMat;
    depthShader.setUniform("mvp", mvp);
    // draw model to depth tex/fb
    model.DrawMinimal(depthShader);
}


Framebuffer CreateDepthAndNormalsFB(f32 width, f32 height) {
    Framebuffer fb;
    fb.size = glm::vec2(width, height);

    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &fb.framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fb.framebufferID));

    // generate our texture that we'll draw to
    GLCall(glGenTextures(1, &fb.texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, fb.texture));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE));

    // using RGBA32F to ensure our depth values don't lose precision when going from the depth buffer
    // to our texture. (normally textures might store the values with less precision than the depth buffer would)
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0));

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
 
    return fb;
}