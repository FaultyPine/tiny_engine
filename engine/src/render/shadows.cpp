#include "shadows.h"

#include "render/shader.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "render/tiny_lights.h"
#include "render/model.h"
#include "tiny_profiler.h"


ShadowMap::ShadowMap(u32 resolution) {
    fb = Framebuffer(resolution, resolution, Framebuffer::FramebufferAttachmentType::DEPTH);
    // this shader renders our scene from the perspective of a light
    depthShader = Shader(ResPath("shaders/depth.vert"), ResPath("shaders/depth.frag"));
}
void ShadowMap::BeginRender() const {
    fb.Bind();
    // completely clear depth texture
    ClearGLBuffers();
    // cull front faces when rendering to depth tex?
    // this theoretically helps peter panning, but some models really
    // aren't meant to be front face culled. This would need to happen on a per-mesh basis which
    // isn't feasible rn
    //glCullFace(GL_FRONT);
}
void ShadowMap::EndRender() const {
    // reset cull mode
    //glCullFace(GL_BACK);
    // bind default fb
    Framebuffer::BindDefaultFrameBuffer();
}

void ShadowMap::RenderShadowCaster(const LightDirectional& light, const Model& model, const Transform& tf) const {
    PROFILE_FUNCTION();
    if (!depthShader.isValid() || !light.enabled) return;
    glm::mat4 lightMat = light.GetLightSpacematrix(); // projection & view
    glm::mat4 modelMat = tf.ToModelMatrix();
    glm::mat4 mvp = lightMat * modelMat;
    depthShader.setUniform("mvp", mvp);
    depthShader.use();
    // draw model to depth tex/fb
    model.DrawMinimal(); 
}
