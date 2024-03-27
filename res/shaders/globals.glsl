

#define MAX_NUM_LIGHTS 4

#include "material.glsl"

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
    int numActiveLights;
    float ambientLightIntensity;
    // TODO: materials
};

vec3 GetViewDir(vec3 fragPosWS) 
{
    return normalize(camPos.xyz - fragPosWS);
}