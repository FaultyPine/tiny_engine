#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 spriteColor;
uniform int isHollow;


void main()
{
    if (isHollow == 1) {
        float edges = 0.97;
        float xright = step(edges, TexCoords.x);
        float ybottom = step(edges, TexCoords.y);
        float xleft = 1-step(1-edges, TexCoords.x);
        float ytop = 1-step(1-edges, TexCoords.y);
        float square = xright + xleft + ybottom + ytop;
        if (square <= 0) discard;
        FragColor = spriteColor;
    }
    else {
        FragColor = spriteColor;
    }
}  