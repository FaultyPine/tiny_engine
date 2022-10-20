#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 spriteColor;
uniform float lineWidth;

void main()
{
    // since our DrawLine func draws a big quad
    // with top-left being the start point and
    // bottom-right being the end point we can just
    // make a diagonal line here

    bool isDiagonal = abs(TexCoords.x - TexCoords.y) < lineWidth;
    if (!isDiagonal) discard;

    FragColor = spriteColor;
}  