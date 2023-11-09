//#include "pch.h"
#include "tiny_lights.h"
#include "tiny_engine.h"
#include "tiny_log.h"
#include "camera.h"
#include "shader.h"
#include "shapes.h"

// TODO: this is hardcoded... ideally this would take a list of transforms and maybe some
// camera frustum info and calculate the optimal size of the projection matrix
glm::mat4 GetDirectionalLightViewProjMatrix(glm::vec3 position, glm::vec3 target) 
{
    const f32 boxScale = 30.0f;
    glm::mat4 lightProj = glm::ortho(-boxScale, boxScale, -boxScale, boxScale, 0.01f, 500.0f);
    glm::mat4 lightView = glm::lookAt(position, target, {0,1,0});
    glm::mat4 lightMat = lightProj * lightView;
    return lightMat;
}

void LightPoint::Visualize()
{
    Shapes3D::DrawCube(Transform(this->position, glm::vec3(0.1)));
}

void LightDirectional::Visualize()
{
    // TODO:
}

LightPoint CreatePointLight(glm::vec3 position, glm::vec4 color, glm::vec3 attenuationParams) 
{
    static u32 global_num_lights = 0;
    TINY_ASSERT(global_num_lights+1 < MAX_NUM_LIGHTS && "Cannot create more then MAX_NUM_LIGHTS lights!");
    LightPoint light = {};
    light.enabled = true;
    light.position = position;
    light.color = color;
    light.constant = attenuationParams.x;
    light.linear = attenuationParams.y;
    light.quadratic = attenuationParams.z;
    light.globalIndex = global_num_lights++;
    // TODO: point light cubemap omnidirectional shadow map
    return light;
}

LightDirectional CreateDirectionalLight(glm::vec3 direction, glm::vec3 position, glm::vec4 color)
{
    LightDirectional light = {};
    light.color = color;
    light.direction = direction;
    light.enabled = true;
    light.lightSpaceMatrix = GetDirectionalLightViewProjMatrix(position, position + direction);
    light.shadowMap = ShadowMap(1024);
    return light;
}


void UpdatePointLightValues(const Shader& shader, const LightPoint& light)
{
    s32 lightIdx = light.globalIndex;
    TINY_ASSERT(lightIdx < MAX_NUM_LIGHTS); 
    shader.use();

    const char* uniformName;
    uniformName = TextFormat("lights[%i].enabled", lightIdx);
    shader.setUniform(uniformName, light.enabled);

    uniformName = TextFormat("lights[%i].position", lightIdx);
    shader.setUniform(uniformName, light.position);

    uniformName = TextFormat("lights[%i].color", lightIdx);
    shader.setUniform(uniformName, light.color);

    uniformName = TextFormat("lights[%i].constant", lightIdx);
    shader.setUniform(uniformName, light.constant);

    uniformName = TextFormat("lights[%i].linear", lightIdx);
    shader.setUniform(uniformName, light.linear);

    uniformName = TextFormat("lights[%i].quadratic", lightIdx);
    shader.setUniform(uniformName, light.quadratic);

    uniformName = TextFormat("lights[%i].shadowMap", lightIdx);
    shader.TryAddSampler(light.shadowMap.id, uniformName);
}

void UpdateSunlightValues(const Shader& shader, const LightDirectional& sunlight)
{
    if (!sunlight.enabled) return;
    shader.use();
    shader.setUniform("sunlight.enabled", sunlight.enabled);
    shader.setUniform("sunlight.direction", sunlight.direction);
    shader.setUniform("sunlight.color", sunlight.color);
    shader.setUniform("sunlight.lightSpaceMatrix", sunlight.lightSpaceMatrix);
    shader.TryAddSampler(sunlight.shadowMap.fb.texture.id, "sunlight.shadowMap");
}