#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalWS;
// Output fragment color
out vec4 finalColor;


// ============================ MATERIALS ==================================

#define NUM_MATERIALS 3
#define MAT_DIFFUSE   0
#define MAT_AMBIENT   1
#define MAT_SPECULAR  2
struct MaterialProperty {
    vec4 color;
    int useSampler;
    sampler2D tex;
};

uniform MaterialProperty materials[NUM_MATERIALS];
uniform float shininess = 16.0;

vec4 GetMaterialColor(int matIdx, vec2 texCoords) {
    int shouldUseSampler = materials[matIdx].useSampler;
    vec4 color = (1-shouldUseSampler) * materials[matIdx].color;
    vec4 tex = shouldUseSampler * texture(materials[matIdx].tex, fragTexCoord);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}

// ================================== LIGHTING ===================================

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

const float AMBIENT_INFLUENCE_DIVISOR = 17.0;
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

vec3 calculateLighting() {
    vec3 lightDot = vec3(0);
    vec3 specular = vec3(0);
    vec3 normal = normalize(fragNormalWS);
    vec3 viewDir = normalize(viewPos - fragPositionWS);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        // TODO: performance boost https://theorangeduck.com/page/avoiding-shader-conditionals
        if (lights[i].enabled == 1) {
            vec3 lightDir = vec3(0.0);

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
                specCo = pow(max(0.0, specularFactor), shininess);
            }
            specular += specCo * lights[i].color.rgb * GetMaterialColor(MAT_SPECULAR, fragTexCoord).rgb;
        }
    }
    return specular * lightDot;
}
// =========================================================================

void main() {
    // Texel color fetching from texture sampler
    vec4 diffuseColor = GetMaterialColor(MAT_DIFFUSE, fragTexCoord);
    vec4 ambient = GetMaterialColor(MAT_AMBIENT, fragTexCoord);

    // colored lighting
    vec4 lighting = vec4(calculateLighting(), 1.0); // lighting always has 1 for alpha


    // set color to our diffuse multiplied by light
    finalColor = diffuseColor * lighting;
    // apply ambient lighting - ambient by itself wouldn't work by itself, we want unlit areas to be a little bit of their real color
    finalColor += diffuseColor*(ambient/AMBIENT_INFLUENCE_DIVISOR);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));
}