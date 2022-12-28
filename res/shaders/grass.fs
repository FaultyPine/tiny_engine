#version 330 core

// Input vertex attributes (from vertex shader)
/*
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalWS;
in vec4 fragPosLightSpace;
flat in int materialId;
*/
// Output fragment color
out vec4 finalColor;

void main() {
    finalColor = vec4(0,1,0,1);
}