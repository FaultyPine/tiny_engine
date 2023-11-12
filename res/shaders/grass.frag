 

// Input vertex attributes (from vertex shader)

in VS_OUT 
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec3 fragNormalOS;
    flat int materialId;
    vec3 fragPositionOS;
} vs_in;
// Output fragment color
out vec4 finalColor;
uniform int numInstances;

#include "easings.glsl"

#include "lighting.glsl"
uniform LightDirectional sunlight;
uniform sampler2D shadowMap;

const vec3 grassBaseColor = vec3(0.05, 0.2, 0.01);
const vec3 grassTipColor = vec3(0.1, 0.5, 0.1);

void main() {
    float height = vs_in.fragTexCoord.y; // 0 at bottom, 1 at tip
    // TODO: blend between dark green at base and more yellowish green at tip
    height = EaseInOutSine(height);
    vec3 col = mix(grassBaseColor, grassTipColor, height);

    // TODO: light these grass blades & use rounded normals

    float shadow = GetDirectionalShadow(
        vs_in.fragPositionWS.xyz, 
        vs_in.fragNormalOS, 
        sunlight);
    // kinda simulates ambient light
    shadow = max(shadow, 0.6); 
    col *= shadow;

    finalColor = vec4(vec3(col), 1.0);
}


