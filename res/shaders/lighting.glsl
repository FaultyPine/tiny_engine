
#include "material.glsl"

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

float PCFShadow(
    vec2 projCoords, 
    float shadowBias, 
    float currentDepth, 
    int resolution, 
    sampler2D shadowMap) 
{
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
float GetShadow(
    vec4 fragPosLS, 
    vec3 lightDir, 
    vec3 normal, 
    sampler2D shadowMap) 
{
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

    float shadow = PCFShadow(projCoords.xy, shadowBias, currentDepth, 1, shadowMap);

    // - bias   gets rid of shadow acne
    //float shadow = currentDepth-shadowBias > shadowMapDepth ? 1.0 : 0.0;
    
    return 1-shadow;
}

void calculateLightingForSingleLight (
    inout vec3 specularLight, 
    inout vec3 diffuseLight,
    in Light light,
    vec3 fragNormalWS,
    vec3 fragPositionWS,
    vec3 viewDir,
    float shininessMaterial,
    vec3 specularMaterial) 
{
    vec3 lightColor = light.color.rgb;
    vec3 lightDir = vec3(0.0);

    if (light.type == LIGHT_DIRECTIONAL) {
        // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
        lightDir = -normalize(light.target - light.position);
    }

    if (light.type == LIGHT_POINT) {
        // distance from the light to our fragment
        lightDir = normalize(light.position - fragPositionWS);
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
        specCo = pow(max(0.0, specularFactor), shininessMaterial);
    }
    specularLight += specCo * lightColor * specularMaterial;
}

vec3 calculateLighting(
    in Material[MAX_NUM_MATERIALS] materials, 
    int materialId,
    vec2 fragTexCoord,
    float ambientLightIntensity,
    in Light lights[MAX_LIGHTS],
    in Light sunlight,
    int numActiveLights,
    vec3 fragNormalWS, 
    vec3 viewDir, 
    sampler2D shadowMap) 
{
    // ambient: if there's a material, tint that material the color of the diffuse and dim it down a lot
    vec3 ambientLight = GetAmbientMaterial(materials, materialId, fragTexCoord).rgb * ambientLightIntensity;

    vec3 diffuseLight = vec3(0);
    vec3 specularLight = vec3(0);
    vec3 sunLightDir = -normalize(sunlight.target - sunlight.position);

    // assumes active lights are at the front of the lights array
    // this is asserted in the model drawing functions
    for (int i = 0; i < numActiveLights; i++) {
        calculateLightingForSingleLight(
            specularLight, 
            diffuseLight,
            lights[i],
            fragNormalWS,
            fragPositionWS,
            viewDir,
            GetShininessMaterial(materials, materialId, fragTexCoord),
            GetSpecularMaterial(materials, materialId, fragTexCoord).rgb);
    }
    // sunlight is seperate from other lights
    calculateLightingForSingleLight(
        specularLight, 
        diffuseLight,
        sunlight,
        fragNormalWS,
        fragPositionWS,
        viewDir,
        GetShininessMaterial(materials, materialId, fragTexCoord),
        GetSpecularMaterial(materials, materialId, fragTexCoord).rgb
    );

    float shadow = GetShadow(fragPosLightSpace, sunLightDir, fragNormalWS, shadowMap);
    vec3 lighting = shadow * (specularLight + diffuseLight);
    lighting += ambientLight; // add ambient on top of everything
    return lighting;
}