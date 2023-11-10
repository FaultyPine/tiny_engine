#ifndef TINY_SHADOWS_H
#define TINY_SHADOWS_H

#include "tiny_defines.h"
#include "render/framebuffer.h"
#include "render/shader.h"

struct LightDirectional;
struct Model;
struct ShadowMap {
    ShadowMap() = default;
    TAPI ShadowMap(u32 resolution);
    void Delete() {
        fb.Delete();
        depthShader.Delete();
    }
    bool isValid() const { return fb.isValid(); }
    TAPI void BeginRender() const;
    // makes the passed in shader "receive" shadows (just gets shadow uniforms set)
    TAPI void ReceiveShadows(Shader& shader, const LightDirectional& light) const;
    // makes passed in model cast shadows (just renders the model into the depth texture)
    TAPI void RenderShadowCaster(const LightDirectional& light, const Model& model, const Transform& tf) const;
    TAPI void EndRender() const;

    Framebuffer fb;
    Shader depthShader;
};



#endif