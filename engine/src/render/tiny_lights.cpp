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
    glm::vec4 color = glm::vec4(1);
    if (!this->enabled) 
    {
        color = glm::vec4(1,1,1,0.3);
    }
    Shapes3D::DrawWireSphere(this->position, 0.1, color);
}

void LightDirectional::Visualize()
{
    // TODO:
}

LightPoint& CreatePointLight(glm::vec3 position, glm::vec4 color, glm::vec3 attenuationParams) 
{
    GlobalLights& lights = GetEngineCtx().lightsSubsystem->lights;
    s32& pointLightIndex = GetEngineCtx().lightsSubsystem->pointLightIndex;
    TINY_ASSERT(pointLightIndex+1 < MAX_NUM_LIGHTS && "Cannot create more then MAX_NUM_LIGHTS lights!");
    LightPoint light = {};
    light.enabled = true;
    light.position = position;
    light.color = color;
    light.constant = attenuationParams.x;
    light.linear = attenuationParams.y;
    light.quadratic = attenuationParams.z;
    // TODO: point light cubemap omnidirectional shadow map

    lights.pointLights[pointLightIndex++] = light;
    return lights.pointLights[pointLightIndex-1];
}

LightDirectional& CreateDirectionalLight(glm::vec3 direction, glm::vec3 position, glm::vec4 color)
{
    GlobalLights& lights = GetEngineCtx().lightsSubsystem->lights;
    LightDirectional light = {};
    light.color = color;
    light.direction = direction;
    light.position = position;
    light.enabled = true;
    light.shadowMap = ShadowMap(1024);

    lights.sunlight = light;
    return lights.sunlight;
}

glm::mat4 LightDirectional::GetLightSpacematrix() const
{
    return GetDirectionalLightViewProjMatrix(position, position + direction);
}

s32 ActiveLightsInFront(LightPoint lights[MAX_NUM_LIGHTS]) {
    s32 numActiveLights = 0;
    for (u32 i = 0; i < MAX_NUM_LIGHTS; i++) {
        if (!lights[i].enabled) {
            for (u32 j = i+1; j < MAX_NUM_LIGHTS; j++) {
                if (lights[i].enabled) {
                    return 0;
                }
            }
        }
        else
        {
            numActiveLights++;
        }
    }
    return numActiveLights;
}

void SetLightingUniforms(const Shader& shader)
{
    GlobalLights& lights = GetEngineCtx().lightsSubsystem->lights;
    s32 numActiveLights = ActiveLightsInFront(lights.pointLights);
    TINY_ASSERT(numActiveLights);
    UpdatePointLightValues(shader, lights.pointLights, numActiveLights);
    UpdateSunlightValues(shader, lights.sunlight);
    shader.setUniform("numActiveLights", numActiveLights);
}

void UpdatePointLightValues(const Shader& shader, LightPoint* lights, u32 numPointLights)
{
    for (u32 i = 0; i < numPointLights; i++)
    {
        LightPoint& light = lights[i];
        if (!light.enabled) continue;

        const char* uniformName;
        uniformName = TextFormat("lights[%i].enabled", i);
        shader.setUniform(uniformName, light.enabled);

        uniformName = TextFormat("lights[%i].position", i);
        shader.setUniform(uniformName, light.position);

        uniformName = TextFormat("lights[%i].color", i);
        shader.setUniform(uniformName, light.color);

        uniformName = TextFormat("lights[%i].constant", i);
        shader.setUniform(uniformName, light.constant);

        uniformName = TextFormat("lights[%i].linear", i);
        shader.setUniform(uniformName, light.linear);

        uniformName = TextFormat("lights[%i].quadratic", i);
        shader.setUniform(uniformName, light.quadratic);

        uniformName = TextFormat("lights[%i].intensity", i);
        shader.setUniform(uniformName, light.intensity);

        uniformName = TextFormat("lights[%i].shadowMap", i);
        shader.TryAddSampler(light.shadowMap, uniformName);
    }
}

void UpdateSunlightValues(const Shader& shader, const LightDirectional& sunlight)
{
    if (!sunlight.enabled) return;
    //shader.use();
    shader.setUniform("sunlight.enabled", sunlight.enabled);
    shader.setUniform("sunlight.direction", sunlight.direction);
    shader.setUniform("sunlight.color", sunlight.color);
    shader.setUniform("sunlight.intensity", sunlight.intensity);
    shader.setUniform("sunlight.lightSpaceMatrix", sunlight.GetLightSpacematrix());
    shader.TryAddSampler(sunlight.shadowMap.fb.texture, "sunlight.shadowMap");
}