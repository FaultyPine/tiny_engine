 
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

#include "globals.glsl"

void main()
{
    // remove translation part of view matrix - centers the skybox cube on the camera
    mat4 noTranslationView = mat4(mat3(view)); 
    // skybox cubemap - vertex positions *are* the uvs
    TexCoords = aPos; 
    vec4 pos = projection * noTranslationView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  