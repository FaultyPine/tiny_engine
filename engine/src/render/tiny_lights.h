#ifndef TINY_LIGHTS_H
#define TINY_LIGHTS_H

//#include "pch.h"
#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "shader.h"
#include "texture.h"
#include "render/framebuffer.h"
#include "render/shadows.h"
#include "render/cubemap.h"

#include "res/shaders/shader_defines.glsl"

struct LightPoint 
{   
    glm::vec3 position = glm::vec3(0);
    s32 enabled = false;
    glm::vec4 color = glm::vec4(1);
    f32 constant = 1.0f;
    f32 linear = 0.09f;
    f32 quadratic = 0.032f;
    f32 intensity = 1.0f;

    TAPI void Visualize();
};

// NOTE: I've been generally assuming we only have one directional light
struct LightDirectional
{
    // directional lights don't really have "positions", but this
    // is still needed for rendering our shadow map
    glm::vec3 position = glm::vec3(0);
    s32 enabled = false;
    glm::vec3 direction = glm::vec3(1);
    f32 intensity = 1.0f;
    glm::vec4 color = glm::vec4(1);

    TAPI void Visualize();
    TAPI glm::mat4 GetLightSpacematrix(glm::mat4* outProj = nullptr, glm::mat4* outView = nullptr) const;
};

struct GlobalLights
{
    LightPoint pointLights[MAX_NUM_LIGHTS] = {};
    LightDirectional sunlight = {};
    u32 GetNumActiveLights();
};

struct LightingSystem
{
    s32 pointLightIndex = 0;
    GlobalLights lights = {};
    f32 ambientLightIntensity = 0.2f;
    ShadowMap directionalShadowMap = {};
    Cubemap pointLightShadowMaps[MAX_NUM_LIGHTS] = {};
};

struct Arena;
void InitializeLightingSystem(Arena* arena);

void SetLightingUniforms(const Shader& shader);

// Create a light and get shader locations
TAPI LightPoint& CreatePointLight(glm::vec3 position, glm::vec4 color, glm::vec3 attenuationParams = glm::vec3(1.0f, 0.09f, 0.032f));
TAPI LightDirectional& CreateDirectionalLight(glm::vec3 direction, glm::vec3 position, glm::vec4 color, f32 intensity);
// Send light properties to shader
void UpdatePointLightValues(const Shader& shader, LightPoint* lights);
void UpdateSunlightValues(const Shader& shader, const LightDirectional& sunlight);

#endif