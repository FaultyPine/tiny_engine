#version 330 core

// Input vertex attributes (from vertex shader)

//in vec3 fragPositionWS;
in vec2 fragTexCoord;
//in vec4 fragColor;
//in vec3 fragNormalWS;
in vec4 fragPosLightSpace;
flat in int materialId;
//in vec3 fragPositionOS;
// Output fragment color
out vec4 finalColor;
uniform int numInstances;

uniform sampler2D depthMap;
float PCFShadow(vec2 projCoords, float shadowBias, float currentDepth, int resolution) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -resolution; x <= resolution; x++) {
        for(int y = -resolution; y <= resolution; y++) {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
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
    float depthMapDepth = texture(depthMap, projCoords.xy).r;
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow

    //float shadow = PCFShadow(projCoords.xy, shadowBias, currentDepth, 1);

    // - bias   gets rid of shadow acne
    float shadow = currentDepth-shadowBias > depthMapDepth ? 1.0 : 0.0;
    
    return 1-shadow;
}

const vec3 brown = vec3(115, 72, 69);


void main() {
    float height = fragTexCoord.y;
    vec3 brown = brown/255.0;
    float shadow = (1-GetShadow()) * 0.1;
    vec3 col = mix(vec3(0,0.5,0), brown, 1-height) - shadow;
    finalColor = vec4(vec3(col), 1.0);
}