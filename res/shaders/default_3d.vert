 
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec2 vertTexCoord;

#include "globals.glsl"

out vec3 normal;
out vec2 texCoord;

uniform mat4 modelMat;

void main()
{
	normal = vertNormal;
	texCoord = vertTexCoord;
	gl_Position = projection * view * modelMat * vec4(vertPos, 1.0);
}