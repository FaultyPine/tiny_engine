 

#include "globals.glsl"


void main() {
    vs_out.fragNormal = vertexNormal;
    gl_Position = projection * view * GetModelMatrix() * vec4(vertexPosition, 1.0);
}