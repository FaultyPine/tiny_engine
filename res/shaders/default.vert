
#include "globals.glsl"

void main(){
	gl_Position = GetProjectionMatrix() * GetViewMatrix() * GetModelMatrix() * vec4(vertexPosition, 1.0);
}