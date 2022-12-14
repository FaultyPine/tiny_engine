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

// ============================ MATERIALS ==================================

#define NUM_MATERIAL_TYPES 4
#define MAX_NUM_MATERIALS 4

struct MaterialProperty {
    vec4 color;
    int useSampler;
    sampler2D tex;
};
struct Material {
    MaterialProperty diffuseMat;
    MaterialProperty ambientMat;
    MaterialProperty specularMat;
    MaterialProperty normalMat;
    float shininess;
};
uniform int useNormalMap = 0; // If unset, use vertex normals. If set, sample normal map
uniform Material materials[MAX_NUM_MATERIALS];
vec4 GetMaterialColor(MaterialProperty mat) {
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, fragTexCoord);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}
vec4 GetDiffuseMaterial() {
    return GetMaterialColor(materials[materialId].diffuseMat);
}
vec4 GetAmbientMaterial() {
    return GetMaterialColor(materials[materialId].ambientMat);
}
vec4 GetSpecularMaterial() {
    return GetMaterialColor(materials[materialId].specularMat);
}
vec4 GetNormalMaterial() {
    return GetMaterialColor(materials[materialId].normalMat);
}

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
uniform vec3 viewPos;
uniform sampler2D depthMap;

vec3 GetNormals() {
    vec3 vertNormals = (1-useNormalMap) * normalize(fragNormalWS);
    vec3 normalMapNormals = (useNormalMap) * GetNormalMaterial().rgb;
    return vertNormals + normalMapNormals;
}

vec3 calculateLighting() {
    vec3 lightDot = vec3(0);
    vec3 specular = vec3(0);
    vec3 normal = GetNormals();
    vec3 viewDir = normalize(viewPos - fragPositionWS);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 1) {
            vec3 lightDir = vec3(0.0);

            // TODO: performance boost https://theorangeduck.com/page/avoiding-shader-conditionals
            if (lights[i].type == LIGHT_DIRECTIONAL) {
                // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
                lightDir = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT) {
                // distance from the light to our fragment
                lightDir = normalize(lights[i].position - fragPositionWS);
            }

            // [0-1] 0 being totally unlit, 1 being in full light
            float NdotL = max(dot(normal, lightDir), 0.0);
            lightDot += lights[i].color.rgb*NdotL;
            float specCo = 0.0;
            if (NdotL > 0.0) {
                // phong
                //vec3 reflectDir = reflect(-lightDir, normal);
                //float specularFactor = dot(viewDir, reflectDir);
                // blinn-phong
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float specularFactor = dot(normal, halfwayDir);
                specCo = pow(max(0.0, specularFactor), materials[materialId].shininess);
            }
            specular += specCo * lights[i].color.rgb * GetSpecularMaterial().rgb;
        }
    }
    return specular * lightDot;
}
// =========================================================================

float GetDepth(vec2 texcoords) {
    float depthMapSample = texture(depthMap, texcoords).r;
    // don't need to linearize this.. even if we were using perspective proj
    // for the shadow map, shadow values are relative when comparing fragment depths
    // linearizing the depth is only useful for debugging perspective projected depth textures
    return depthMapSample;
}

// 0 is in shadow, 1 is out of shadow
float GetShadow(vec4 fragPosLS) {
    const float shadowBias = 0.005;
    //vec3 normal = GetNormals();
    //vec3 lightDir = 
    
    // manual perspective divide
    // range [-1,1]
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) // if fragment in light space is outside the frustum, it should be fully lit
        return 1.0;

    // depth value from shadow map
    float depthMapDepth = GetDepth(projCoords.xy);
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow
    // - bias   gets rid of shadow acne
    float shadow = currentDepth-shadowBias > depthMapDepth ? 1.0 : 0.0;
    return 1-shadow;
}

void main() {
    float shadow = GetShadow(fragPosLightSpace);

    // Texel color fetching from texture sampler
    vec4 diffuseColor = GetDiffuseMaterial();
    vec4 ambient = GetAmbientMaterial();

    // colored lighting
    vec4 lighting = vec4(calculateLighting(), 1.0); // lighting always has 1 for alpha


    // set color to our diffuse multiplied by light
    finalColor = diffuseColor * lighting * vec4(vec3(shadow), 1.0);
    // apply ambient lighting - we want unlit areas to be just a little bit of their real color
    finalColor += diffuseColor*(ambient/17.0);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));
}