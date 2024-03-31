
#include "globals.glsl"

uniform mat4 modelMat;

void main(){
	gl_Position = projection * view * modelMat * vec4(vertexPosition, 1.0);
}