#pragma once

#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "tiny_lights.h" //MAX_NUM_LIGHTS
#include "tiny_material.h"

#include "res/shaders/shader_defines.glsl"


// only included in shader.cpp

// std140 alignment rules
TALIGN(16) struct UBOGlobals
{
    glm::vec4 screenSize;
    // camera
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec4 camPos;
    glm::vec4 camFront;

    //                  16 aligned bytes
    f32 nearClip;
    f32 farClip;
    f32 FOV;
    // misc
    f32 time;
    //                  ----------
    
    // lighting
    struct LightDirectionalUBO
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec4 direction; // intensity in alpha
        glm::vec4 color;
    };

    struct LightPointUBO
    {
        glm::vec4 position; // unused alpha
        glm::vec4 color;
        glm::vec4 attenuationParams; // {constant, linear, quadratic, light intensity}
    };
    LightDirectionalUBO sunlight;
    LightPointUBO lights[MAX_NUM_LIGHTS];
    glm::vec4 numActiveLightsAndAmbientIntensity;

    // ----- per object data -----
    struct GPUPerObjectData
    {
        glm::mat4 modelMat;
        glm::mat4 normalMat;
    };

    GPUPerObjectData objectData[MAX_NUM_OBJECTS];
};

struct ShaderBufferGlobals
{
    // only a single uniform buffer, contain all global shader data
    u32 uboObject = 0; 
    UBOGlobals globals = {};
};

// initializes buffer on gpu
void InitializeUBOs(ShaderBufferGlobals& globals);
// called just before the game starts rendering a frame
void ShaderSystemPreDraw(ShaderBufferGlobals& globals);
// binds all used UBO indices to the given shader program, called just after shader initialization
void HandleShaderUBOInit(u32 shaderProgram);
