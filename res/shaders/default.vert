
#include "globals.glsl"

void main(){
	gl_Position = projection * view * modelMat * vec4(vertexPosition, 1.0);
}