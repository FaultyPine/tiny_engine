//#ifdef TINY_GLOBALS_GLSL
//define TINY_GLOBALS_GLSL

#include "shader_defines.glsl"

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

#ifndef NO_VS_OUT
// Output vertex attributes (to fragment shader)
out VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragVertexColor;
    vec3 fragNormal; // OS
    vec3 fragTangent; // OS
    flat uint objectID;
} vs_out;
#endif

mat4 GetModelMatrix();
mat3 GetNormalMatrix();
void VertexToFrag()
{
    vs_out.fragPositionWS = vec3(GetModelMatrix()*vec4(vertexPosition, 1.0));
    vs_out.fragTexCoord = vertexTexCoord;
    vs_out.fragVertexColor = vertexColor;
    vs_out.fragNormal = vec3(GetNormalMatrix()*vertexNormal);
    vs_out.fragTangent = vertexTangent;
    vs_out.objectID = objectID;
}

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

#ifndef NO_VS_OUT
// Input vertex attributes (from vertex shader)
in VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragVertexColor;
    vec3 fragNormal;
    vec3 fragTangent;
    flat uint objectID;
} vs_in;
#endif

layout (location = 0) out vec4 fragColor;

#endif // FRAGMENT_SHADER


struct ObjectData
{
    mat4 modelMat;
    mat4 normalMat;
};

uint GetObjectID()
{
    uint result = 0;
    #ifdef VERTEX_SHADER
    result = objectID;
    #endif
    #if defined(FRAGMENT_SHADER) && !defined(NO_VS_OUT)
    result = vs_in.objectID;
    #endif
    return result;
}

struct LightDirectional 
{
    mat4 projection;
    mat4 view;
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
    vec4 screenSize;
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

    ObjectData objectData[MAX_NUM_OBJECTS];
};

uniform bool isDirectionalShadowPass = false;

mat4 GetProjectionMatrix()
{
    return isDirectionalShadowPass ? sunlight.projection : projection;
}

mat4 GetViewMatrix()
{
    return isDirectionalShadowPass ? sunlight.view : view;
}

mat4 GetModelMatrix()
{
    uint idx = GetObjectID();
    // objectID is an EntityRef which is generally just a hash
    return objectData[idx % MAX_NUM_OBJECTS].modelMat;
}
mat3 GetNormalMatrix()
{
    uint idx = GetObjectID();
    // objectID is an EntityRef which is generally just a hash
    return mat3(objectData[idx % MAX_NUM_OBJECTS].normalMat);
}

int GetNumActiveLights()
{
    return int(activeLightsAndAmbientIntensity.x);
}
float GetAmbientLightIntensity()
{
    return activeLightsAndAmbientIntensity.y;
}

#ifdef FRAGMENT_SHADER
vec2 GetScreenUVs()
{
    vec2 screenUV = gl_FragCoord.xy/screenSize.xy;
    return screenUV;
}
#endif

vec3 GetViewDir(vec3 fragPosWS) 
{
    return normalize(camPos.xyz - fragPosWS);
}

vec2 GetTexelSize(in sampler2D tex)
{
    return 1.0 / textureSize(tex, 0);
}

//#endif