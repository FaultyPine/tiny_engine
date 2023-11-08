#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalWS;
in vec4 fragPosLightSpace;
flat in int materialId;
// Output fragment color
out vec4 finalColor;

uniform float nearClip;
uniform float farClip;
uniform int shouldCrosshatch = 0;
#include "crosshatch.glsl"

// ============================ MATERIALS ==================================
#include "material.glsl"
uniform int useNormalMap = 0; // If unset, use vertex normals. If set, sample normal map
uniform Material materials[MAX_NUM_MATERIALS];

// ================================== LIGHTING ===================================

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};
// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform int numActiveLights;
uniform vec3 viewPos;
uniform float ambientLightIntensity = 0.15;

vec3 GetViewDir() {
    return normalize(viewPos - fragPositionWS);
}
vec3 GetNormals() {
    vec3 vertNormals = (1-useNormalMap) * normalize(fragNormalWS);
    vec3 normalMapNormals = (useNormalMap) * GetNormalMaterial(materials, materialId, fragTexCoord).rgb;
    return vertNormals + normalMapNormals;
}

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
float GetShadow(vec4 fragPosLS, vec3 lightDir, vec3 normal) {
    //const float shadowBias = 0.005;
    // maximum bias of 0.05 and a minimum of 0.005 based on the surface's normal and light direction
    float shadowBias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);  
    // manual perspective divide
    // range [-1,1]
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) // if fragment in light space is outside the frustum, it should be fully lit
        return 1.0;


    // depth value from shadow map
    //float shadowMapDepth = texture(shadowMap, projCoords.xy).r;
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow

    float shadow = PCFShadow(projCoords.xy, shadowBias, currentDepth, 1);

    // - bias   gets rid of shadow acne
    //float shadow = currentDepth-shadowBias > shadowMapDepth ? 1.0 : 0.0;
    
    return 1-shadow;
}

vec3 calculateLighting(vec3 fragNormalWS, vec3 viewDir) {
    // ambient: if there's a material, tint that material the color of the diffuse and dim it down a lot
    vec3 ambientLight = GetAmbientMaterial(materials, materialId, fragTexCoord).rgb * ambientLightIntensity;

    vec3 diffuseLight = vec3(0);
    vec3 specularLight = vec3(0);
    // direction of directional light (if there are multiple, this currently uses the last)
    vec3 sunLightDir = vec3(0);

    // assumes active lights are at the front of the lights array
    // this is asserted in the model drawing functions
    for (int i = 0; i < numActiveLights; i++) {
        vec3 lightColor = lights[i].color.rgb;
        vec3 lightDir = vec3(0.0);

        if (lights[i].type == LIGHT_DIRECTIONAL) {
            // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
            lightDir = -normalize(lights[i].target - lights[i].position);
            // grab last (hopefully only) directional light direction for use in shadow calculations
            sunLightDir = lightDir;
        }

        if (lights[i].type == LIGHT_POINT) {
            // distance from the light to our fragment
            lightDir = normalize(lights[i].position - fragPositionWS);
        }

        // Diffuse light
        // [0-1] 0 being totally unlit, 1 being in full light
        float NdotL = max(dot(fragNormalWS, lightDir), 0.0);
        diffuseLight += lightColor * NdotL;

        // specular light
        float specCo = 0.0;
        if (NdotL > 0.0) {
            // blinn-phong
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float specularFactor = dot(fragNormalWS, halfwayDir);
            specCo = pow(max(0.0, specularFactor), GetShininessMaterial(materials, materialId, fragTexCoord));
        }
        specularLight += specCo * lightColor * GetSpecularMaterial(materials, materialId, fragTexCoord).rgb;
    }

    float shadow = GetShadow(fragPosLightSpace, sunLightDir, fragNormalWS);
    vec3 lighting = shadow * (specularLight + diffuseLight);
    lighting += ambientLight; // add ambient on top of everything
    return lighting;
}
// =========================================================================


void main() {
    // Texel color fetching from texture sampler
    vec3 diffuseColor = GetDiffuseMaterial(materials, materialId, fragTexCoord).rgb;

    // colored lighting
    vec3 lighting = calculateLighting(GetNormals(), GetViewDir());
    vec3 col = lighting * diffuseColor;
    float alpha = 1.0;

    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    finalColor = vec4(col, alpha);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));

    //finalColor = vec4(normalize(fragNormalWS), 1.0);
}
