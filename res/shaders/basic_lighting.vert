 

// Input vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexTexCoord;
layout (location = 4) in vec4 vertexColor;

#include "globals.glsl"

uniform mat4 modelMat;
uniform mat3 normalMat;

// Output vertex attributes (to fragment shader)
out VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormal; // OS
    vec3 fragTangent; // OS
} vs_out;


void main()
{
    // transform model space vertexPosition into world space by multiplying with model matrix
    vs_out.fragPositionWS = vec3(modelMat*vec4(vertexPosition, 1.0));
    vs_out.fragTexCoord = vertexTexCoord;
    vs_out.fragColor = vertexColor;
    vs_out.fragNormal = vec3(normalMat*vertexNormal);
    vs_out.fragTangent = normalMat*vertexTangent;
    gl_Position = projection * view * modelMat*vec4(vertexPosition, 1.0);
}