 

// Input vertex attributes (from vertex shader)
in VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormalWS;
    flat int materialId;
} vs_in;

#include "globals.glsl"

// Output fragment color
out vec4 finalColor;

#include "material.glsl"
#include "lighting.glsl"

vec3 GetViewDir() {
    return normalize(camPos - vs_in.fragPositionWS);
}
vec3 GetNormals() {
    vec3 vertNormals = (1-useNormalMap) * normalize(vs_in.fragNormalWS);
    vec3 normalMapNormals = (useNormalMap) * GetNormalMaterial(materials, vs_in.materialId, vs_in.fragTexCoord).rgb;
    return vertNormals + normalMapNormals;
}

// =========================================================================


void main() {
    // base diffuse material texture
    vec3 diffuseColor = GetDiffuseMaterial(materials, vs_in.materialId, vs_in.fragTexCoord).rgb;

    // colored lighting
    vec3 lighting = calculateLighting(
        materials,
        vs_in.materialId,
        vs_in.fragTexCoord,
        GetNormals(), 
        GetViewDir(),
        vs_in.fragPositionWS);
    vec3 col = lighting * diffuseColor;
    float alpha = 1.0;

    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    finalColor = vec4(col, alpha);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
