#version 330 core

// Input vertex attributes (from vertex shader)

//in vec3 fragPositionWS;
in vec2 fragTexCoord;
//in vec4 fragColor;
//in vec3 fragNormalWS;
//in vec4 fragPosLightSpace;
//flat in int materialId;
//in vec3 fragPositionOS;
// Output fragment color
out vec4 finalColor;
flat in int instanceID;
uniform int numInstances;


const vec3 brown = vec3(115, 72, 69);


void main() {
    float height = fragTexCoord.y;
    vec3 brown = brown/255.0;
    vec3 col = mix(vec3(0,1,0), brown, 1-height);
    finalColor = vec4(col, 1.0);
}