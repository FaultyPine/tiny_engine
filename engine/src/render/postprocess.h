#pragma once

#include "tiny_defines.h"

struct Arena;
struct Shader;
struct Framebuffer;

struct PostprocessSettings
{
    struct SSAOSettings
    {
        f32 occlusionPower = 1.0;
        f32 sampleRadius = 0.5;
    };
    SSAOSettings ssaoSettings = {};
};

namespace Postprocess
{

TAPI void SetPostprocessShader(const Shader& shader);
TAPI void ApplySSAOUniforms(const Shader& shader);
TAPI PostprocessSettings& ModifySettings();

void InitializePostprocessing(Arena* arena);
TAPI void PostprocessFramebuffer(const Framebuffer& fbToProcess, const Shader& shader);
TAPI void PostprocessFramebuffer(const Framebuffer& fbToProcess, const Framebuffer& dst, const Shader& shader);
TAPI Framebuffer* GetPostprocessingFramebuffer();
TAPI Shader* GetPostprocessingShader();

} // namespace Postprocess
