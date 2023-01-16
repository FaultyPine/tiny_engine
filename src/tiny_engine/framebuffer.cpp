#include "framebuffer.h"
#include "tiny_engine.h"

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
    s32 component = type == DEPTH ? GL_DEPTH_COMPONENT : GL_RGB;
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    if (type == DEPTH) {
        // if its a depth texture, everything outside our texture should default to 1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        f32 borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    }
    else {
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); 
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); 
    }
    // generate texture to attach to framebuffer
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, component, width, height, 0, component, type == DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL));
    if (type == DEPTH) {
        // disable color buffer if it's a depth tex
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    // attach texture to framebuffer
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, texture, 0));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));  
}


static const char* depthFragShaderStr = R"(
#version 330 core
void main() {
    // this happens implicitly, explicitly putting this here for clarity
    gl_FragDepth = gl_FragCoord.z;
}
)";
static const char* depthVertShaderStr = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
}  
)";


ShadowMap::ShadowMap(u32 resolution) {
    fb = Framebuffer(resolution, resolution, Framebuffer::FramebufferAttachmentType::DEPTH);
    // this shader renders our scene from the perspective of a light
    depthShader = Shader::CreateShaderFromStr(depthVertShaderStr, depthFragShaderStr);
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
void ShadowMap::SetShadowUniforms(Shader& shader, const Light& light) const {
    shader.use();
    shader.TryAddSampler(fb.GetTexture().id, "depthMap");
    shader.ActivateSamplers();
    shader.setUniform("lightSpaceMatrix", light.GetLightViewProjMatrix());
}
void ShadowMap::RenderToShadowMap(const Light& light, Model& model, const Transform& tf) const {
    depthShader.use();
    glm::mat4 lightMat = light.GetLightViewProjMatrix();
    glm::mat4 modelMat = tf.ToModelMatrix();
    glm::mat4 mvp = lightMat * modelMat;
    SetShadowUniforms(model.cachedShader, light);
    // draw model to depth tex/fb
    model.Draw(depthShader, mvp, modelMat);
}