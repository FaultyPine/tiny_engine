 
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertTexCoords;

#include "globals.glsl"

uniform mat4 modelMat;
out vec2 TexCoords;
out vec3 Normal;
void main(){
    Normal = vertNormal;
    TexCoords = vertTexCoords;
	gl_Position = projection * view * modelMat * vec4(vertPos, 1.0);
}