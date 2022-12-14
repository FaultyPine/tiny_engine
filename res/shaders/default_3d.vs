#version 330 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec2 vertTexCoord;
layout (location = 3) in vec3 vertColor;

out vec3 normal;
out vec2 texCoord;
out vec3 color;


uniform mat4 mvp;

void main()
{
	normal = vertNormal;
	texCoord = vertTexCoord;
    color = vertColor;
	gl_Position = mvp * vec4(vertPos, 1.0);
}