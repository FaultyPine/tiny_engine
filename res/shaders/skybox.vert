 

out vec3 TexCoords;

#include "globals.glsl"

void main()
{
    // remove translation part of view matrix - centers the skybox cube on the camera
    mat4 noTranslationView = mat4(mat3(GetViewMatrix())); 
    // skybox cubemap - vertex positions *are* the uvs
    TexCoords = vertexPosition; 
    vec4 pos = GetProjectionMatrix() * noTranslationView * vec4(vertexPosition, 1.0);
    gl_Position = pos.xyww;
}  