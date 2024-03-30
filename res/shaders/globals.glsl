

#ifdef VERTEX_SHADER

#ifndef CUSTOM_VERTEX_DATA
// vertex data
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexTexCoord;
layout (location = 4) in vec4 vertexColor;
layout (location = 5) in uint objectID;
layout (location = 6) in uint materialID;
layout (location = 7) in mat4 instanceModelMat;
#endif

// Output vertex attributes (to fragment shader)
out VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragVertexColor;
    vec3 fragNormal; // OS
    vec3 fragTangent; // OS
    flat uint objectID;
    flat uint materialID;
} vs_out;

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
    vs_out.materialID = materialID;
}

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
    flat uint objectID;
    flat uint materialID;
} vs_in;

layout (location = 0) out vec4 fragColor;

#endif // FRAGMENT_SHADER

#define MAX_NUM_OBJECTS 1000
#define MAX_NUM_MATERIALS 500

struct ObjectData
{
    mat4 modelMat;
    mat4 normalMat;
};

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

    MaterialPacked materials[MAX_NUM_MATERIALS];
    ObjectData objectData[MAX_NUM_OBJECTS];
};

uint GetObjectID()
{
    uint result = 0;
    #ifdef VERTEX_SHADER
    result = objectID;
    #endif
    #ifdef FRAGMENT_SHADER
    result = vs_in.objectID;
    #endif
    return result;
}
uint GetMaterialID()
{
    uint result = 0;
    #ifdef VERTEX_SHADER
    result = materialID;
    #endif
    #ifdef FRAGMENT_SHADER
    result = vs_in.materialID;
    #endif
    return result;
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

MaterialPacked GetMaterial()
{
    uint idx = GetMaterialID();
    // % here unnecessary..? materialID isn't a hash, its an actual index
    return materials[idx % MAX_NUM_MATERIALS];
}

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