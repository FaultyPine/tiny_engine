 
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 fragTexCoord;

uniform mat4 spriteModelMat;
uniform mat4 spriteViewMat;
uniform mat4 spriteProjectionMat;

void main()
{
    vec2 uv = vertex.zw;
    vec2 pos = vertex.xy;

    fragTexCoord = uv;
    gl_Position = spriteProjectionMat * spriteViewMat * spriteModelMat * vec4(pos, 0.0, 1.0);
}