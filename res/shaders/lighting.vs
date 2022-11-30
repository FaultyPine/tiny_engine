#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec4 vertexColor;
layout (location = 4) in int  vertexMaterialId;

// Input uniform values
uniform mat4 mvp;
uniform mat4 modelMat;

// Output vertex attributes (to fragment shader)
out vec3 fragPositionWS;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormalWS;
flat out int materialId;


void main()
{
    // transform model space vertexPosition into world space by multiplying with model matrix
    fragPositionWS = vec3(modelMat*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    materialId = vertexMaterialId;

    // normals are in model space... transform them to world space here
    // TODO: in the future might want to do this operation on the CPU
    // since inversing matrices is pretty expensive to do for every vertex
    mat3 matNormal = mat3(transpose(inverse(modelMat)));
    fragNormalWS = normalize(vec3(matNormal*vertexNormal));

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}