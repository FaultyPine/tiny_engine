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

uniform sampler2D shadowMap;
float PCFShadow(vec2 projCoords, float shadowBias, float currentDepth, int resolution) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -resolution; x <= resolution; x++) {
        for(int y = -resolution; y <= resolution; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= float(pow(resolution*2 + 1, 2));
    return shadow;
}

// 0 is in shadow, 1 is out of shadow
float GetShadow() {
    const float shadowBias = 0.005;
    // maximum bias of 0.05 and a minimum of 0.005 based on the surface's normal and light direction
    //float shadowBias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);  
    // manual perspective divide
    // range [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) // if fragment in light space is outside the frustum, it should be fully lit
        return 1.0;


    // depth value from shadow map
    float shadowMapDepth = texture(shadowMap, projCoords.xy).r;
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow

    //float shadow = PCFShadow(projCoords.xy, shadowBias, currentDepth, 1);

    // - bias   gets rid of shadow acne
    float shadow = currentDepth-shadowBias > shadowMapDepth ? 1.0 : 0.0;
    
    return 1-shadow;
}

const vec3 grassBaseColor = vec3(0.05, 0.2, 0.01);
const vec3 grassTipColor = vec3(0.1, 0.5, 0.1);

void main() {
    float height = fragTexCoord.y; // 0 at bottom, 1 at tip
    // TODO: blend between dark green at base and more yellowish green at tip
    height = EaseInOutSine(height);
    vec3 col = mix(grassBaseColor, grassTipColor, height);

    // TODO: light these grass blades & use rounded normals

    // don't want shadows making it completely black
    // this kinda simulates ambient light
    float shadow = max(GetShadow(), 0.4); 
    col *= shadow;

    finalColor = vec4(vec3(col), 1.0);
}


