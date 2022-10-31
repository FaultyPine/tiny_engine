#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec4 spriteColor;

void main()
{
    FragColor = spriteColor * texture(image, TexCoords);
    if (FragColor.a == 0) {
        discard;
    }
}  