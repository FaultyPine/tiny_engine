 

#include "globals.glsl"

#include "easings.glsl"
#include "lighting.glsl"

const vec3 grassBaseColor = vec3(0.05, 0.2, 0.01);
const vec3 grassTipColor = vec3(0.1, 0.5, 0.1);

void main() {
    vec2 texCoord = vs_in.fragTexCoord;
    float height = vs_in.fragTexCoord.y; // 0 at bottom, 1 at tip
    // TODO: blend between dark green at base and more yellowish green at tip
    height = EaseInOutSine(height);
    vec3 diffuse = mix(grassBaseColor, grassTipColor, height);

    vec4 albedoSpec = GetAlbedoSpec(texCoord);
    vec3 diffuseMaterial = albedoSpec.rgb;
    float shininess = albedoSpec.a;
    vec4 depthNormals = GetDepthNormals(texCoord);
    vec3 normalWS = depthNormals.rgb;
    float depth = depthNormals.a;
    vec3 fragPositionWS = GetPositionWS(texCoord);
    vec3 viewDir = GetViewDir(fragPositionWS);

    // colored lighting
    vec3 lighting = calculateLighting(
        texCoord,
        -sunlight.direction.xyz, 
        -sunlight.direction.xyz,
        vs_in.fragPositionWS,
        diffuse,
        shininess);

    //float shadow = GetDirectionalShadow(
    //    vs_in.fragPositionWS.xyz,
    //    vs_in.fragNormal);
    //shadow = max(0.6, shadow); // lil ambient light type thing
    //vec3 col = diffuse*shadow;
    vec3 col = diffuse * lighting;

    fragColor = vec4(vec3(col), 1.0);
}


