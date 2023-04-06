#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    vec2 uv = vertex.zw;
    vec2 pos = vertex.xy;

    TexCoords = uv;
    gl_Position = projection * model * vec4(pos, 0.0, 1.0);
}