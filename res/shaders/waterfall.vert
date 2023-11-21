 

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;

#include "globals.glsl"

out vec3 Normal;
out vec2 texCoord;

uniform mat4 modelMat;

void main() {
    Normal = vertexNormal;
    texCoord = vertexTexCoord;
    gl_Position = projection * view * modelMat * vec4(vertexPosition, 1.0);
}