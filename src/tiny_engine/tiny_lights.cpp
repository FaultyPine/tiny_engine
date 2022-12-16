#include "tiny_lights.h"
#include "tiny_engine.h"
#include "camera.h"

glm::mat4 Light::GetLightViewProjMatrix() const {
    const f32 boxScale = 15.0f;
    glm::mat4 lightProj = glm::ortho(-boxScale, boxScale, -boxScale, boxScale, Camera::GetMainCamera().nearClip, Camera::GetMainCamera().farClip);
    glm::mat4 lightView = glm::lookAt(position, target, {0,1,0});
    glm::mat4 lightMat = lightProj * lightView;
    return lightMat;
}

static u32 global_num_lights = 0;

Light CreateLight(s32 type, glm::vec3 position, glm::vec3 target, glm::vec4 color) {
    Light light = {};

    light.enabled = true;
    light.type = type;
    light.position = position;
    light.target = target;
    light.color = color;
    light.globalIndex = global_num_lights++;

    return light;
}

void UpdateLightValues(Shader shader, Light light) {
    s32 lightIdx = light.globalIndex;
    // TODO: make a better check?
    // maybe prevent creating more than maximum # of lights?
    ASSERT(lightIdx < MAX_NUM_LIGHTS); 
    shader.use();
    // Send to shader light enabled state and type
    const char* enabledLoc = TextFormat("lights[%i].enabled", lightIdx);
    shader.setUniform(enabledLoc, light.enabled);

    const char* typeLoc = TextFormat("lights[%i].type", lightIdx);
    shader.setUniform(typeLoc, light.type);

    // Send to shader light position values
    const char* positionLoc = TextFormat("lights[%i].position", lightIdx);
    shader.setUniform(positionLoc, light.position);

    // Send to shader light target position values
    const char* targetLoc = TextFormat("lights[%i].target", lightIdx);
    shader.setUniform(targetLoc, light.target);

    // Send to shader light color values
    const char* colorLoc = TextFormat("lights[%i].color", lightIdx);
    shader.setUniform(colorLoc, light.color);
}