 

// Input vertex attributes (from vertex shader)
in VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormal;
} vs_in;
out vec4 finalColor;

#include "globals.glsl"
#include "material.glsl"
#include "lighting.glsl"

// =========================================================================


void main() {
    // base diffuse material texture
    vec3 diffuseColor = GetDiffuseMaterial(vs_in.fragTexCoord).rgb;

    // colored lighting
    vec3 lighting = calculateLighting(
        material,
        vs_in.fragTexCoord,
        GetNormals(vs_in.fragNormal, vs_in.fragTexCoord), 
        GetViewDir(vs_in.fragPositionWS),
        vs_in.fragPositionWS);
    vec3 col = lighting * diffuseColor;
    float alpha = 1.0;

    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    finalColor = vec4(col, alpha);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
