#version 330 core
out vec4 color;

in vec2 TexCoords;
in vec4 ParticleColor;

uniform sampler2D mainTex;

void main()
{
    color = (texture(mainTex, TexCoords) * ParticleColor);
}  