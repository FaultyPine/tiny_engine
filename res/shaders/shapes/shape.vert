 
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

//#define CUSTOM_VERTEX_DATA
//#include "globals.glsl"
//#undef CUSTOM_VERTEX_DATA
out vec2 fragTexCoord;
uniform mat4 modelMat;

void main()
{
    vec2 uv = vertex.zw;
    vec2 pos = vertex.xy;
    fragTexCoord = uv;
    gl_Position = projection * modelMat * vec4(pos, 0.0, 1.0);
}