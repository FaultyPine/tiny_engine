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

#define MAX_NUM_LIGHTS 4


struct LightPoint 
{   
    bool enabled = true;
    glm::vec3 position = glm::vec3(0);
    glm::vec4 color = glm::vec4(1);
    f32 constant = 1.0f;
    f32 linear = 0.09f;
    f32 quadratic = 0.032f;
    f32 intensity = 1.0f;
    Cubemap shadowMap;
    s32 globalIndex = -1;

    TAPI void Visualize();
};
struct LightDirectional
{
    bool enabled = false;
    // directional lights don't really have "positions", but this
    // is still needed for rendering our shadow map
    glm::vec3 position = glm::vec3(0);
    glm::vec3 direction = glm::vec3(1);
    glm::vec4 color = glm::vec4(1);
    f32 intensity = 1.0f;
    //glm::mat4 lightSpaceMatrix = glm::mat4(1);
    ShadowMap shadowMap;
    TAPI void Visualize();
    TAPI glm::mat4 GetLightSpacematrix() const;
};

// Create a light and get shader locations
TAPI LightPoint CreatePointLight(glm::vec3 position, glm::vec4 color, glm::vec3 attenuationParams = glm::vec3(1.0f, 0.09f, 0.032f));
TAPI LightDirectional CreateDirectionalLight(glm::vec3 direction, glm::vec3 position, glm::vec4 color);
// Send light properties to shader
void UpdatePointLightValues(const Shader& shader, const LightPoint& light);
void UpdateSunlightValues(const Shader& shader, const LightDirectional& sunlight);

#endif