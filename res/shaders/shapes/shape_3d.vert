 
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec2 vertTexCoords;
uniform mat4 mvp;
out vec2 TexCoords;
out vec3 Normal;
void main(){
    Normal = vertNormal;
    TexCoords = vertTexCoords;
	gl_Position = mvp * vec4(vertPos, 1.0);
}