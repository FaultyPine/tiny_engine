#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;
uniform vec4 color = vec4(1);

void main()
{
	FragColor = color;
}