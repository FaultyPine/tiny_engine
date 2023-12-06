//#include "pch.h"
#include "tiny_lights.h"
#include "tiny_engine.h"
#include "tiny_log.h"
#include "camera.h"
#include "shader.h"
#include "shapes.h"
#include "render/shadows.h"
#include "tiny_alloc.h"

void InitializeLightingSystem(Arena* arena)
{
    EngineContext& ctx = GetEngineCtx();
    ctx.lightsSubsystem = (LightingSystem*)arena_alloc(arena, sizeof(LightingSystem));
    *ctx.lightsSubsystem = {};
}

static BoundingBox GetTightBoundsOnCamFrustum(
    glm::mat4 camViewMatrix, 
    glm::vec3 lightDir, 
    glm::mat4 camProjMatrix, 
    glm::vec3& outLightWorldPos, 
    void* outOrthoProjInfo)
{

}

// TODO: this is hardcoded... ideally this would take a list of transforms and maybe some
// camera frustum info and calculate the optimal size of the projection matrix
static glm::mat4 GetDirectionalLightViewProjMatrix(glm::vec3 position, glm::vec3 target) 
{
    const f32 boxScale = 30.0f;
    glm::mat4 lightProj = glm::ortho(-boxScale, boxScale, -boxScale, boxScale, 0.01f, 500.0f);
    glm::mat4 lightView = glm::lookAt(position, target, {0,1,0});
    glm::mat4 lightMat = lightProj * lightView;
    return lightMat;
}

void LightPoint::Visualize()
{
    if (!this->enabled) return;
    glm::vec4 color = glm::vec4(1);
    Shapes3D::DrawWireSphere(this->position, 0.1, color);
}

void LightDirectional::Visualize()
{
    Shapes3D::DrawWireSphere(this->position, 5.0f, glm::vec4(1));
    Shapes3D::DrawLine(this->position, this->position + (this->direction * 10.0f), glm::vec4(1), 2.0f);
}

LightPoint& CreatePointLight(glm::vec3 position, glm::vec4 color, glm::vec3 attenuationParams) 
{
    LightingSystem& lightingSystem = *GetEngineCtx().lightsSubsystem;
    GlobalLights& lights = lightingSystem.lights;
    s32& pointLightIndex = lightingSystem.pointLightIndex;
    TINY_ASSERT(pointLightIndex+1 < MAX_NUM_LIGHTS && "Cannot create more then MAX_NUM_LIGHTS lights!");
    LightPoint light = {};
    light.enabled = true;
    light.position = position;
    light.color = color;
    light.constant = attenuationParams.x;
    light.linear = attenuationParams.y;
    light.quadratic = attenuationParams.z;
    // TODO: point light cubemap omnidirectional shadow map
    //lightingSystem.pointLightShadowMaps[pointLightIndex] = Cubemap();

    lights.pointLights[pointLightIndex++] = light;
    return lights.pointLights[pointLightIndex-1];
}

LightDirectional& CreateDirectionalLight(glm::vec3 direction, glm::vec3 position, glm::vec4 color, f32 intensity)
{
    LightingSystem& lightingSystem = *GetEngineCtx().lightsSubsystem;
    GlobalLights& lights = lightingSystem.lights;
    LightDirectional light = {};
    light.color = color;
    light.direction = direction;
    light.position = position;
    light.intensity = intensity;
    light.enabled = true;
    lightingSystem.directionalShadowMap = ShadowMap(2048);

    lights.sunlight = light;
    return lights.sunlight;
}

glm::mat4 LightDirectional::GetLightSpacematrix() const
{
    return GetDirectionalLightViewProjMatrix(position, position + direction);
}

bool ActiveLightsInFront(LightPoint lights[MAX_NUM_LIGHTS]) {
    for (u32 i = 0; i < MAX_NUM_LIGHTS; i++) {
        if (!lights[i].enabled) {
            for (u32 j = i+1; j < MAX_NUM_LIGHTS; j++) {
                if (lights[i].enabled) {
                    return false;
                }
            }
        }
    }
    return true;
}

u32 GlobalLights::GetNumActiveLights()
{
    s32 numActiveLights = 0;
    for (u32 i = 0; i < MAX_NUM_LIGHTS; i++) {
        if (pointLights[i].enabled) {
            numActiveLights++;
        }
    }
    return numActiveLights;
}

void SetLightingUniforms(const Shader& shader)
{
    LightingSystem& lightSystem = *GetEngineCtx().lightsSubsystem;
    GlobalLights& lights = lightSystem.lights;
    TINY_ASSERT(ActiveLightsInFront(lights.pointLights));
    // these only set sampler uniforms. The rest are set in our UBO (shader.cpp rn)
    UpdatePointLightValues(shader, lights.pointLights);
    UpdateSunlightValues(shader, lights.sunlight);
}

void UpdatePointLightValues(const Shader& shader, LightPoint* lights)
{
    Cubemap* pointLightShadowMaps = &GetEngineCtx().lightsSubsystem->pointLightShadowMaps[0];
    for (u32 i = 0; i < MAX_NUM_LIGHTS; i++)
    {
        LightPoint& light = lights[i];
        if (!light.enabled) continue;
        shader.TryAddSampler(pointLightShadowMaps[i], TextFormat("pointLightShadowMaps[%i]", i));
    }
}

void UpdateSunlightValues(const Shader& shader, const LightDirectional& sunlight)
{
    if (sunlight.enabled) 
    {
        ShadowMap& sunlightShadowMap = GetEngineCtx().lightsSubsystem->directionalShadowMap;
        shader.TryAddSampler(sunlightShadowMap.fb.texture, "directionalLightShadowMap");
    }
}