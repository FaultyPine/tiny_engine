 

#include "globals.glsl"


void main()
{
    VertexToFrag();
    gl_Position = projection * view * GetModelMatrix()*vec4(vertexPosition, 1.0);
}