#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec4 vertexColor;
layout (location = 4) in int  vertexMaterialId;

// Input uniform values
uniform mat4 mvp;
uniform mat4 modelMat;
uniform mat3 normalMat;
//uniform mat4 lightSpaceMatrix;

// Output vertex attributes (to fragment shader)
out VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormalWS;
    flat int materialId;
} vs_out;


void main()
{
    // transform model space vertexPosition into world space by multiplying with model matrix
    vs_out.fragPositionWS = vec3(modelMat*vec4(vertexPosition, 1.0));
    vs_out.fragTexCoord = vertexTexCoord;
    vs_out.fragColor = vertexColor;
    // world space frag pos to light space
    //fragPosLightSpace = lightSpaceMatrix * vec4(fragPositionWS, 1.0);
    vs_out.materialId = vertexMaterialId;
    vs_out.fragNormalWS = vec3(normalMat*vertexNormal);

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}