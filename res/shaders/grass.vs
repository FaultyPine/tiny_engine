#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec4 vertexColor;
layout (location = 4) in int  vertexMaterialId;

// Input uniform values
uniform mat4 instanceMvps[100];
uniform int numInstances;

// Output vertex attributes (to fragment shader)
/*
out vec3 fragPositionWS;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormalWS;
out vec4 fragPosLightSpace;
flat out int materialId;
*/

void main()
{
    /*
    // transform model space vertexPosition into world space by multiplying with model matrix
    fragPositionWS = vec3(modelMat*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    // world space frag pos to light space
    //fragPosLightSpace = lightSpaceMatrix * vec4(fragPositionWS, 1.0);
    materialId = vertexMaterialId;
    fragNormalWS = normalize(vec3(normalMat*vertexNormal));
    */
    
    mat4 mvp = instanceMvps[gl_InstanceID];
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}