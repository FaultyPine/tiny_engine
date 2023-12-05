 

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;

#include "globals.glsl"

uniform mat4 modelMat;

out VS_OUT 
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormal;
    vec3 fragTangent;
    vec3 fragPositionOS;
} vs_out;

void main() {
    vs_out.fragNormal = vertexNormal;
    gl_Position = projection * view * modelMat * vec4(vertexPosition, 1.0);
}