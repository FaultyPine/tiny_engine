#pragma once

#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "tiny_lights.h" //for MAX_NUM_LIGHTS

// only included in shader.cpp

struct UBOGlobals
{
    // NOTE: ***MUST*** follow std140 alignment rules (I.E. have no implicit padding).
    // to keep it simple, imagine the only acceptable data types are scalars and vectors of 2 or 4 size.
    // matrices must be 4x4 or 2x2, never 3 - same with vectors. 
    // if you want to use a vec3, make sure there's a 4 byte padding or some other 4 byte value directly after it
    // although it would be better to pack that scalar into a vec4 with some other vec3

    // NOTE: keep in sync with uniform block (currently) in globals.glsl

    /*
You never use vec3. Note that this applies to matrices that have 3 columns/rows (depending on your matrix orientation).
You alignas/_Alignas your vector members properly. vec2 objects must be 8-byte aligned, and vec4 must be 16-byte aligned. Note that this applies to matrix types that have 2 or 4 columns/rows too.

C's _Alignas feature only applies to variable declarations, so you can't apply it to the type. You have to put it on each variable as you use it.

Under std140, you never make arrays of anything that isn't a vec4 or equivalent.
Under std140, struct members of a block should be aligned to 16-bytes
    */

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
        glm::mat4 lightSpaceMatrix;
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

    // TODO: materials

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
