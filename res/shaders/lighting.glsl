
#include "common.glsl"
#include "material.glsl"

// Gbuffer
uniform sampler2D depthNormals;
uniform sampler2D albedoSpec;
uniform usampler2D objMatId;
uniform sampler2D gbuf_positionWS;

vec4 GetDepthNormals(vec2 uv)
{
    return texture(depthNormals, uv);
}
vec4 GetAlbedoSpec(vec2 uv)
{
    return texture(albedoSpec, uv);
}
uvec2 GetObjMatId(vec2 uv)
{
    return texture(objMatId, uv).rg;
}
vec3 GetPositionWS(vec2 uv)
{
    return texture(gbuf_positionWS, uv).rgb;
}

// samplers cannot be stored in ubo/ssbo buffers.... :/
uniform samplerCube pointLightShadowMaps[MAX_NUM_LIGHTS];
uniform sampler2D directionalLightShadowMap;
uniform sampler2D aoTexture;
uniform samplerCube skybox;

// TODO: poisson disk sampling for less bandy smoother shadow edges
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
            bool isInShadow = currentDepth > shadowMapDepth;
            shadow += isInShadow ? 1.0 : 0.0;
        }    
    }
    // average shadow across samples
    shadow /= float(pow(resolution * 2 + 1, 2));
    return shadow;
}

mat4 GetDirectionalLightspaceMatrix()
{
    return sunlight.projection * sunlight.view;
}

float GetDirectionalShadow(
    vec3 fragPosWS, 
    vec3 fragNormalWS) 
{
    vec3 lightDir = -sunlight.direction.xyz; // points towards-pixel-from-light, so reverse it to get from-pixel-to-light
    // the light space matrix contains the view and projection matrices     
    // for our light (view representing where our light is "looking") and projection representing the shadow frustum
    vec4 fragPosLS = GetDirectionalLightspaceMatrix() * vec4(fragPosWS, 1.0);
    // bias based on the surface's normal and light direction
    float shadowBias = max(0.065 * (1.0 - dot(fragNormalWS, lightDir)), 0.005);
    // manual perspective divide - now we're in NDC
    // range [-1,1]
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) // if fragment in light space is outside the frustum, it should be fully lit
        return 0.0;
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow
    float shadow = PCFShadow(projCoords.xy, currentDepth - shadowBias, 6, directionalLightShadowMap);
    // reverse the above so when we multiply with lighting, more shadows darkens the color
    return 1.0-shadow;
}

void calculateLightingForPointLight (
    inout vec3 specularLight, 
    inout vec3 diffuseLight,
    in LightPoint light,
    vec3 fragNormalWS,
    vec3 fragPositionWS,
    vec3 viewDir,
    float shininess) 
{
    float lightIntensity = light.attenuationParams.a;
    vec3 lightColor = light.color.rgb * lightIntensity;
    vec3 lightPos = light.position.xyz;
    // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
    vec3 lightDir = normalize(lightPos - fragPositionWS);

    // Diffuse light
    // [0-1] 0 being totally unlit, 1 being in full light
    //float NdotL = max(dot(fragNormalWS, lightDir), 0.0);
    float NdotL = 1/max(distance(fragPositionWS, lightPos), EPSILON);
    vec3 currentLightDiffuse = lightColor * NdotL;

    // specular light
    float specCo = 0.0;
    if (NdotL > 0.0) {
        // blinn-phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularFactor = dot(fragNormalWS, halfwayDir);
        specCo = pow(max(0.0, specularFactor), max(EPSILON, shininess));
    }
    vec3 currentLightSpecular = specCo * lightColor; //* specularMaterial;

    // dist from current fragment to light source
    // TODO: https://lisyarus.github.io/blog/graphics/2022/07/30/point-light-attenuation.html
    // attentuation: https://www.desmos.com/calculator/5eaerimi8q
    float distance = length(lightPos - fragPositionWS);
    vec4 attenuationParams = light.attenuationParams;
    float constant = attenuationParams.r;
    float linear = attenuationParams.g;
    float quadratic = attenuationParams.b;
    float attenuation = 1.0 / (constant + linear * distance + 
    		                    quadratic * (distance * distance));

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
    float shininess) 
{
    float lightIntensity = light.direction.a; // intensity is in alpha of direction
    // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
    vec3 lightDir = normalize(-light.direction.xyz);
    vec3 lightColor = light.color.rgb * lightIntensity; 

    // Diffuse light
    // [0-1] 0 being totally unlit, 1 being in full light
    float NdotL = max(dot(fragNormalWS, lightDir), 0.0);
    vec3 currentLightDiffuse = lightColor * NdotL;

    // specular light
    float specCo = 0.0;
    if (NdotL > 0.0) 
    {
        // blinn-phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularFactor = max(0.0, dot(fragNormalWS, halfwayDir));
        specCo = pow(max(0.0, specularFactor), max(EPSILON, shininess));
    }
    vec3 currentLightSpecular = specCo * lightColor; // * specularMaterial; // tint specular color?
    float shadow = GetDirectionalShadow(fragPositionWS, fragNormalWS);

    diffuseLight += currentLightDiffuse * shadow;
    specularLight += currentLightSpecular * shadow;
}

vec3 calculateLighting(
    vec2 fragTexCoord,
    vec3 fragNormalWS, 
    vec3 viewDir, 
    vec3 fragPositionWS,
    vec3 diffuseMaterial,
    float shininess)
{
    // ambient: if there's a material, tint that material the color of the diffuse and dim it down a lot
    vec3 ambientLight = diffuseMaterial * GetAmbientLightIntensity();
    ambientLight *= texture(aoTexture, GetScreenUVs()).rgb;

    vec3 diffuseLight = vec3(0);
    vec3 specularLight = vec3(0);

    // sunlight is seperate from other lights
    calculateLightingForDirectionalLight(
        specularLight, 
        diffuseLight,
        sunlight,
        fragNormalWS,
        fragPositionWS,
        viewDir,
        shininess);
    // assumes active lights are at the front of the lights array
    for (int i = 0; i < GetNumActiveLights(); i++) 
    {
        calculateLightingForPointLight(
            specularLight, 
            diffuseLight,
            lights[i],
            fragNormalWS,
            fragPositionWS,
            viewDir,
            shininess);
    }

    vec3 lighting = diffuseLight + specularLight + ambientLight;
    return lighting;
}