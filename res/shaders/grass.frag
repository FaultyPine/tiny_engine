#version 330 core

// Input vertex attributes (from vertex shader)

in vec3 fragPositionWS;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormalOS;
in vec4 fragPosLightSpace;
flat in int materialId;
in vec3 fragPositionOS;
// Output fragment color
out vec4 finalColor;
uniform int numInstances;

#include "easings.glsl"

#include "lighting.glsl"
uniform Light sunlight;
uniform sampler2D shadowMap;

const vec3 grassBaseColor = vec3(0.05, 0.2, 0.01);
const vec3 grassTipColor = vec3(0.1, 0.5, 0.1);

void main() {
    float height = fragTexCoord.y; // 0 at bottom, 1 at tip
    // TODO: blend between dark green at base and more yellowish green at tip
    height = EaseInOutSine(height);
    vec3 col = mix(grassBaseColor, grassTipColor, height);

    // TODO: light these grass blades & use rounded normals

    float shadow = GetShadow(
        fragPosLightSpace, 
        -normalize(sunlight.target - sunlight.position), 
        fragNormalOS, 
        shadowMap);
    // kinda simulates ambient light
    shadow = max(shadow, 0.6); 
    col *= shadow;

    finalColor = vec4(vec3(col), 1.0);
}


