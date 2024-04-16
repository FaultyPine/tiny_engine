
#include "globals.glsl"

uniform mat4 modelMat;

void main(){
	gl_Position = GetProjectionMatrix() * GetViewMatrix() * modelMat * vec4(vertexPosition, 1.0);
}