 
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

#define CUSTOM_VERTEX_DATA
#include "globals.glsl"
#undef CUSTOM_VERTEX_DATA

uniform mat4 spriteModelMat;
uniform mat4 spriteViewMat;
uniform mat4 spriteProjectionMat;

void main()
{
    vec2 uv = vertex.zw;
    vec2 pos = vertex.xy;

    vs_out.fragTexCoord = uv;
    gl_Position = spriteProjectionMat * spriteViewMat * spriteModelMat * vec4(pos, 0.0, 1.0);
}