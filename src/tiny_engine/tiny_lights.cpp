#include "tiny_lights.h"
#include "tiny_engine/tiny_engine.h"

static u32 global_num_lights = 0;

Light CreateLight(s32 type, glm::vec3 position, glm::vec3 target, glm::vec4 color, Shader shader) {
    Light light = {};

    light.enabled = true;
    light.type = type;
    light.position = position;
    light.target = target;
    light.color = color;
    light.globalIndex = global_num_lights++;

    UpdateLightValues(shader, light);

    return light;
}

void UpdateLightValues(Shader shader, Light light) {
    shader.use();
    // Send to shader light enabled state and type
    const char* enabledLoc = TextFormat("lights[%i].enabled", light.globalIndex);
    shader.setUniform(enabledLoc, light.enabled);

    const char* typeLoc = TextFormat("lights[%i].type", light.globalIndex);
    shader.setUniform(typeLoc, light.type);

    // Send to shader light position values
    const char* positionLoc = TextFormat("lights[%i].position", light.globalIndex);
    shader.setUniform(positionLoc, light.position);

    // Send to shader light target position values
    const char* targetLoc = TextFormat("lights[%i].target", light.globalIndex);
    shader.setUniform(targetLoc, light.target);

    // Send to shader light color values
    const char* colorLoc = TextFormat("lights[%i].color", light.globalIndex);
    shader.setUniform(colorLoc, light.color);
}