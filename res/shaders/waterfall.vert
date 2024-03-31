 

#include "globals.glsl"


void main() {
    vs_out.fragNormal = vertexNormal;
    vs_out.fragTexCoord = vertexTexCoord;
    gl_Position = projection * view * GetModelMatrix() * vec4(vertexPosition, 1.0);
}