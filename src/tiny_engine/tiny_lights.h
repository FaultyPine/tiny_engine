#ifndef TINY_LIGHTS_H
#define TINY_LIGHTS_H

#include "pch.h"
#include "shader.h"

// needed for 3d light visualization
#include "shapes.h"

#define MAX_NUM_LIGHTS 4

enum LightType {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1
};

struct Light {   
    s32 type = 0;
    bool enabled = true;
    glm::vec3 position = glm::vec3(5);
    glm::vec3 target = glm::vec3(0);
    glm::vec4 color = glm::vec4(1);
    f32 attenuation = 1.0;
    s32 globalIndex = -1;

    void Visualize() {
        Shapes3D::DrawCube(position, glm::vec3(0.2), 0, {0,1,0}, type == LightType::LIGHT_DIRECTIONAL ? glm::vec4(1,1,0,1) : glm::vec4(0.8));
    }
};

// Create a light and get shader locations
Light CreateLight(s32 type, glm::vec3 position, glm::vec3 target, glm::vec4 color);
// Send light properties to shader
void UpdateLightValues(Shader shader, Light light);

#endif