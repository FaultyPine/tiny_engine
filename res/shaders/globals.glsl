

layout (std140) uniform Globals
{
    // camera
    mat4 projection;
    mat4 view;
    vec3 camPos;
    float nearClip;
    vec3 camFront;
    float farClip;
    float FOV;

    // lighting

    // misc
    
};