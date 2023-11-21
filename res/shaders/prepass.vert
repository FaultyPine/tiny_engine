 

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

#include "globals.glsl"

uniform mat4 modelMat;

out vec3 fragNormalOS;

void main() {
    fragNormalOS = vertexNormal;
    gl_Position = projection * view * modelMat * vec4(vertexPosition, 1.0);
}