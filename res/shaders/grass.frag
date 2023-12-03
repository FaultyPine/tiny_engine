 

in VS_OUT 
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragTangent;
    vec3 fragPositionOS;
} vs_in;
out vec4 finalColor;

#include "easings.glsl"
#include "globals.glsl"
#include "lighting.glsl"
#include "material.glsl"

const vec3 grassBaseColor = vec3(0.05, 0.2, 0.01);
const vec3 grassTipColor = vec3(0.1, 0.5, 0.1);

void main() {
    float height = vs_in.fragTexCoord.y; // 0 at bottom, 1 at tip
    // TODO: blend between dark green at base and more yellowish green at tip
    height = EaseInOutSine(height);
    vec3 diffuse = mix(grassBaseColor, grassTipColor, height);

    vec3 normalWS = GetNormals(vs_in.fragNormal, vs_in.fragTangent, vs_in.fragTexCoord);
    vec3 viewDir = GetViewDir(vs_in.fragPositionWS);
    // colored lighting
    vec3 lighting = calculateLighting(
        material,
        vs_in.fragTexCoord,
        -sunlight.direction, 
        -sunlight.direction,
        vs_in.fragPositionWS);

    //float shadow = GetDirectionalShadow(
    //    vs_in.fragPositionWS.xyz,
    //    vs_in.fragNormal);
    //shadow = max(0.6, shadow); // lil ambient light type thing
    //vec3 col = diffuse*shadow;
    vec3 col = diffuse * lighting;

    finalColor = vec4(vec3(col), 1.0);
}


