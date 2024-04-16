#include "postprocess.h"

#include "tiny_engine.h"
#include "mem/tiny_arena.h"
#include "shader.h"
#include "framebuffer.h"
#include <random>


struct SSAOData
{
    Texture ssaoNoise;
};

struct PostprocessingSystem
{
    Shader postprocessShader = {};
    Framebuffer postprocessFramebuffer = {};
    SSAOData ssao = {};
    PostprocessSettings settings = {};
};

PostprocessingSystem& GetPP()
{
    return *GetEngineCtx().postprocessingSystem;
}

namespace Postprocess
{


void InitializePostprocessing(Arena* arena)
{
    PostprocessingSystem* pp = arena_alloc_type(arena, PostprocessingSystem, 1);
    *pp = {};
    GetEngineCtx().postprocessingSystem = pp;
    pp->postprocessFramebuffer = Framebuffer(Camera::GetScreenWidth(), Camera::GetScreenHeight(), 1, false);
}

PostprocessSettings& ModifySettings()
{
    return GetPP().settings;
}

void PostprocessFramebuffer(
    const Framebuffer& fbToProcess, 
    const Shader& shader)
{
    Postprocess::PostprocessFramebuffer(fbToProcess, GetPP().postprocessFramebuffer, shader);
}

void PostprocessFramebuffer(const Framebuffer& fbToProcess, const Framebuffer& dst, const Shader& shader)
{
    PostprocessingSystem& pp = GetPP();
    if (!shader.isValid() || !dst.isValid())
    {
        return;
    }
    // apply settings (TODO: make this automatic depending on variables in settings struct (macro magic))
    shader.setUniform("ssaoSampleRadius", pp.settings.ssaoSettings.sampleRadius);
    shader.setUniform("occlusionPower", pp.settings.ssaoSettings.occlusionPower);
    // ------
    dst.Bind();
    ClearGLBuffers(); // NOTE: need to clear framebuffers when drawing to them for the first time
    // using our "postprocessing" framebuffer as a temporary to render to/blit back from
    fbToProcess.DrawToFramebuffer(dst, FramebufferAttachmentType::COLOR0, shader);
}

void ApplySSAOUniforms(const Shader& shader)
{
    PostprocessingSystem& pp = GetPP();

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    #define SSAO_KERNEL_SIZE 64
    for (u32 i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        /*
        glm::vec3 sample(
            GetRandomf(-1.0f, 1.0f),
            GetRandomf(-1.0f, 1.0f),
            GetRandomf(0.0f, 1.0f) // this gives samples in a unit hemisphere facing positive Z
        );
        */
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator)
        );
        sample  = glm::normalize(sample);
        //sample *= GetRandomf(0.0f, 1.0f);
        sample *= randomFloats(generator);
        // weight samples to be closer to the origin
        f32 scale = (f32)i / (f32)SSAO_KERNEL_SIZE; 
        scale   = Math::Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        shader.setUniform(TextFormat("ssaoKernel[%i]", i), sample);
    }
    if (!pp.ssao.ssaoNoise.isValid())
    {
        // random kernel rotations
        #define SSAO_NOISE_SIZE 16
        glm::vec3 noiseData[SSAO_NOISE_SIZE];
        for (u32 i = 0; i < SSAO_NOISE_SIZE; i++)
        {
            glm::vec3 noise(
                //GetRandomf(-1.0f, 1.0f),
                //GetRandomf(-1.0f, 1.0f),
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f); 
            noiseData[i] = noise;
        }
        u32 noiseDimension = sqrt(SSAO_NOISE_SIZE);
        TextureProperties properties = TextureProperties::RGBA_NEAREST();
        properties.texWrapMode = TextureProperties::TexWrapMode::REPEAT;
        properties.texFormat = TextureProperties::TexFormat::RGB16F;
        properties.imgFormat = TextureProperties::ImageFormat::RGB;
        properties.imgDataType = TextureProperties::ImageDataType::FLOAT;
        u32 hash = HashBytes((u8*)noiseData, sizeof(glm::vec3) * SSAO_NOISE_SIZE);
        pp.ssao.ssaoNoise = LoadGPUTextureFromImg((u8*)noiseData, noiseDimension, noiseDimension, properties, hash);
    }
    shader.TryAddSampler(pp.ssao.ssaoNoise, "ssaoNoise");
}

void SetPostprocessShader(const Shader& shader)
{
    GetPP().postprocessShader = shader;
}

Framebuffer* GetPostprocessingFramebuffer()
{
    return &GetPP().postprocessFramebuffer;
}

Shader* GetPostprocessingShader()
{
    return &GetPP().postprocessShader;
}

} // namespace Postprocess