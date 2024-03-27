

#ifdef VERTEX_SHADER

#ifndef CUSTOM_VERTEX_DATA
// vertex data
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexTexCoord;
layout (location = 4) in vec4 vertexColor;
layout (location = 5) in uint objectID;
layout (location = 6) in mat4 instanceModelMat;
#endif

// Output vertex attributes (to fragment shader)
out VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragVertexColor;
    vec3 fragNormal; // OS
    vec3 fragTangent; // OS
} vs_out;

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

// Input vertex attributes (from vertex shader)
in VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragVertexColor;
    vec3 fragNormal;
    vec3 fragTangent;
} vs_in;

layout (location = 0) out vec4 fragColor;

#endif // FRAGMENT_SHADER

uniform mat4 modelMat;
uniform mat3 normalMat;


#include "material.glsl"
#define MAX_NUM_LIGHTS 4
struct LightDirectional 
{
    mat4 lightSpaceMatrix;
    vec4 direction; // intensity in alpha
    vec4 color;
};

struct LightPoint
{
    vec4 position; // unused alpha
    vec4 color;
    vec4 attenuationParams; // {constant, linear, quadratic, light intensity}
};

layout (std140) buffer Globals
{
    // camera
    mat4 projection;
    mat4 view;
    vec4 camPos;
    vec4 camFront;

    float nearClip;
    float farClip;
    float FOV;
    // misc
    float time;

    // lighting
    LightDirectional sunlight;
    LightPoint lights[MAX_NUM_LIGHTS];
    vec4 activeLightsAndAmbientIntensity; // x -> numActiveLights (cast this to int), y -> ambientLightIntensity
    // TODO: materials

};

int GetNumActiveLights()
{
    return int(activeLightsAndAmbientIntensity.x);
}
float GetAmbientLightIntensity()
{
    return activeLightsAndAmbientIntensity.y;
}

vec3 GetViewDir(vec3 fragPosWS) 
{
    return normalize(camPos.xyz - fragPosWS);
}