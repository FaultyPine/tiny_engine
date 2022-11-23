#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalWS;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

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
uniform vec4 ambient;
uniform float shininess = 16.0;

void calculateLighting(inout vec3 lightDot, inout vec3 specular) {
    vec3 normal = normalize(fragNormalWS);
    vec3 viewDir = normalize(viewPos - fragPositionWS);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        // possible performance boost? https://theorangeduck.com/page/avoiding-shader-conditionals
        if (lights[i].enabled == 1)
        {
            vec3 lightDir = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                // target - position is direction from target pointing towards the light, inverse that to get proper lit parts of mesh
                lightDir = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
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
            vec3 coloredSpecular = specCo * lights[i].color.rgb;
            specular += coloredSpecular;
        }
    }
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    vec3 specular = vec3(0.0);
    vec3 lightDot = vec3(0.0);
    // gets our COLORED light and specular values
    calculateLighting(lightDot, specular);
    
    // colored lighting
    vec4 lighting = vec4(specular * lightDot, 1.0); // lighting always has 1 for alpha

    // TODO: ??? why set it then add the texel color again
    finalColor = texelColor * lighting;
    //finalColor += texelColor*(ambient/10.0)*colDiffuse;
    finalColor += texelColor*ambient*colDiffuse;



    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));
}