
#include "material.glsl"

#define     MAX_LIGHTS              4

struct LightDirectional 
{
    int enabled;
    vec3 direction;
    vec4 color;
    float intensity;
    mat4 lightSpaceMatrix;
    sampler2D shadowMap;
};

struct LightPoint
{
    int enabled;
    vec3 position;
    vec4 color;
    float constant;
    float linear;
    float quadratic;
    float intensity;
    samplerCube shadowMap;
};

float PCFShadow(
    vec2 projCoords, 
    float currentDepth, 
    int resolution, 
    sampler2D shadowMap) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -resolution; x <= resolution; x++) {
        for(int y = -resolution; y <= resolution; y++) {
            vec2 shadowMapUVOffset = vec2(x, y) * texelSize;
            float shadowMapDepth = texture(shadowMap, projCoords.xy + shadowMapUVOffset).r; 
            shadow += currentDepth > shadowMapDepth ? 1.0 : 0.0;        
        }    
    }
    // average shadow across samples
    shadow /= float(pow(resolution * 2 + 1, 2));
    return shadow;
}

// 0 is in shadow, 1 is out of shadow
float GetDirectionalShadow(
    vec3 fragPosWS, 
    vec3 fragNormalWS,
    in LightDirectional light) 
{
    vec3 lightDir = -light.direction; // points towards pixel from light, so reverse it to get pixel to light
    // the light space matrix contains the view and projection matrices 
    // for our light (view representing where our light is "looking") and projection representing the shadow frustum
    vec4 fragPosLS = light.lightSpaceMatrix * vec4(fragPosWS, 1.0);

    //const float shadowBias = 0.005;
    // bias based on the surface's normal and light direction
    //float shadowBias = max(0.01 * (1.0 - dot(fragNormalWS, -light.direction)), 0.005);  
    float shadowBias = mix(0.005, 0.0, dot(fragNormalWS, -light.direction));  
    // manual perspective divide - now we're in NDC
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
    float shadow = PCFShadow(projCoords.xy, currentDepth - shadowBias, 2, light.shadowMap);

    // - bias   gets rid of shadow acne
    //float shadow = currentDepth-shadowBias > shadowMapDepth ? 1.0 : 0.0;
    
    return 1.0-shadow;
}

void calculateLightingForPointLight (
    inout vec3 specularLight, 
    inout vec3 diffuseLight,
    in LightPoint light,
    vec3 fragNormalWS,
    vec3 fragPositionWS,
    vec3 viewDir,
    float shininessMaterial,
    vec3 specularMaterial) 
{
    vec3 lightColor = light.color.rgb;
    // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
    vec3 lightDir = normalize(light.position - fragPositionWS);

    // Diffuse light
    // [0-1] 0 being totally unlit, 1 being in full light
    float NdotL = max(dot(fragNormalWS, lightDir), 0.0);
    vec3 currentLightDiffuse = lightColor * NdotL;

    // specular light
    float specCo = 0.0;
    if (NdotL > 0.0) {
        // blinn-phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularFactor = dot(fragNormalWS, halfwayDir);
        specCo = pow(max(0.0, specularFactor), shininessMaterial);
    }
    vec3 currentLightSpecular = specCo * lightColor * specularMaterial;

    // dist from current fragment to light source
    float distance = length(light.position - fragPositionWS) / light.intensity;
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));

    diffuseLight += currentLightDiffuse * attenuation;
    specularLight += currentLightSpecular * attenuation;
}

void calculateLightingForDirectionalLight (
    inout vec3 specularLight, 
    inout vec3 diffuseLight,
    in LightDirectional light,
    vec3 fragNormalWS,
    vec3 fragPositionWS,
    vec3 viewDir,
    float shininessMaterial,
    vec3 specularMaterial) 
{
    vec3 lightColor = light.color.rgb;
    // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
    vec3 lightDir = normalize(-light.direction);

    // Diffuse light
    // [0-1] 0 being totally unlit, 1 being in full light
    float NdotL = max(dot(fragNormalWS, lightDir), 0.0);
    vec3 currentLightDiffuse = lightColor * NdotL;

    // specular light
    float specCo = 0.0;
    if (NdotL > 0.0) {
        // blinn-phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularFactor = dot(fragNormalWS, halfwayDir);
        specCo = pow(max(0.0, specularFactor), shininessMaterial);
    }
    vec3 currentLightSpecular = specCo * lightColor * specularMaterial;

    diffuseLight += currentLightDiffuse * light.intensity;
    specularLight += currentLightSpecular * light.intensity;
}

vec3 calculateLighting(
    in Material[MAX_NUM_MATERIALS] materials, 
    int materialId,
    vec2 fragTexCoord,
    float ambientLightIntensity,
    in LightPoint lights[MAX_LIGHTS],
    in LightDirectional sunlight,
    int numActiveLights,
    vec3 fragNormalWS, 
    vec3 viewDir, 
    vec3 fragPositionWS)
{
    // ambient: if there's a material, tint that material the color of the diffuse and dim it down a lot
    vec3 ambientLight = GetAmbientMaterial(materials, materialId, fragTexCoord).rgb * ambientLightIntensity;

    vec3 diffuseLight = vec3(0);
    vec3 specularLight = vec3(0);
    float shininess = GetShininessMaterial(materials, materialId, fragTexCoord);
    vec3 specularMaterial = GetSpecularMaterial(materials, materialId, fragTexCoord).rgb;
    float shadow = GetDirectionalShadow(fragPositionWS, fragNormalWS, sunlight);

    // sunlight is seperate from other lights
    calculateLightingForDirectionalLight(
        specularLight, 
        diffuseLight,
        sunlight,
        fragNormalWS,
        fragPositionWS,
        viewDir,
        shininess,
        specularMaterial);
    // assumes active lights are at the front of the lights array
    // this is asserted in the model drawing functions
    for (int i = 0; i < numActiveLights; i++) {
        calculateLightingForPointLight(
            specularLight, 
            diffuseLight,
            lights[i],
            fragNormalWS,
            fragPositionWS,
            viewDir,
            shininess,
            specularMaterial);
    }

    vec3 lighting = specularLight + diffuseLight;
    lighting *= shadow;
    lighting += ambientLight; // add ambient on top of everything
    return lighting;
}