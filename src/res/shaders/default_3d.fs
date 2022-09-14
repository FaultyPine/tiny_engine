#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;
in vec3 color;


// texture samplers
uniform sampler2D tex_diffuse1;
uniform sampler2D tex_diffuse2;
uniform float time;

void main()
{
	FragColor = vec4(normal, 1.0);
}