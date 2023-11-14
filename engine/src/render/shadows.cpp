#include "shadows.h"

#include "render/shader.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "render/tiny_lights.h"
#include "model.h"


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
void ShadowMap::ReceiveShadows(Shader& shader, const LightDirectional& light) const {
    if (!shader.isValid() || !light.enabled) return;
    shader.TryAddSampler(fb.GetTexture(), "shadowMap");
    shader.setUniform("lightSpaceMatrix", light.GetLightSpacematrix());
}
void ShadowMap::RenderShadowCaster(const LightDirectional& light, const Model& model, const Transform& tf) const {
    if (!depthShader.isValid() || !light.enabled) return;
    glm::mat4 lightMat = light.GetLightSpacematrix();
    glm::mat4 modelMat = tf.ToModelMatrix();
    glm::mat4 mvp = lightMat * modelMat;
    depthShader.setUniform("mvp", mvp);
    // draw model to depth tex/fb
    model.DrawMinimal(depthShader, tf);
}
