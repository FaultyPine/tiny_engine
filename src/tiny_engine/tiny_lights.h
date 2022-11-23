#ifndef TINY_LIGHTS_H
#define TINY_LIGHTS_H

#include "pch.h"
#include "shader.h"

struct Light {   
    s32 type = 0;
    bool enabled = true;
    glm::vec3 position = glm::vec3(5);
    glm::vec3 target = glm::vec3(0);
    glm::vec4 color = glm::vec4(1);
    f32 attenuation = 1.0;
    s32 globalIndex = -1;
};

enum LightType {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1
};

// Create a light and get shader locations
Light CreateLight(s32 type, glm::vec3 position, glm::vec3 target, glm::vec4 color, Shader shader);
// Send light properties to shader
void UpdateLightValues(Shader shader, Light light);

#endif