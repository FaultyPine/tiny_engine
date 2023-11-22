

layout (std140) uniform Globals
{
    // camera
    mat4 projection;
    mat4 view;
    vec4 camPos;
    vec4 camFront;
    float nearClip;
    float farClip;
    float FOV;
    // misc
    float time;

    // TODO: lighting
};