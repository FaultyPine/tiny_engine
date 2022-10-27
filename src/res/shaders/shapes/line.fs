#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 spriteColor;

void main()
{
    FragColor = spriteColor;
}  