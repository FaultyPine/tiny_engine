 

#include "globals.glsl"


void main()
{
    // transform model space vertexPosition into world space by multiplying with model matrix
    vs_out.fragPositionWS = vec3(modelMat*vec4(vertexPosition, 1.0));
    vs_out.fragTexCoord = vertexTexCoord;
    vs_out.fragVertexColor = vertexColor;
    vs_out.fragNormal = vec3(normalMat*vertexNormal);
    vs_out.fragTangent = vertexTangent;
    gl_Position = projection * view * modelMat*vec4(vertexPosition, 1.0);
}