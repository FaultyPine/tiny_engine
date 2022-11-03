#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D mainTex;
uniform vec4 color;

void main()
{
    FragColor = color * texture(mainTex, TexCoords);
    if (FragColor.a == 0) {
        discard;
    }
}  