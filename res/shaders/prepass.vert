 

#include "globals.glsl"


void main() {
    VertexToFrag();
    gl_Position = GetProjectionMatrix() * GetViewMatrix() * GetModelMatrix() * vec4(vertexPosition, 1.0);
}