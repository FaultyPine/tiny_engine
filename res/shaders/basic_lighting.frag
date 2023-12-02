 

// Input vertex attributes (from vertex shader)
in VS_OUT
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormal;
    vec3 fragTangent;
} vs_in;
out vec4 finalColor;

#include "globals.glsl"
#include "material.glsl"
#include "lighting.glsl"

// =========================================================================


void main() 
{
    vec2 texCoord = vs_in.fragTexCoord;
    if (GetOpacityMaterial(texCoord).r < 0.2)
    {
        discard;
    }
    // base diffuse material texture
    vec3 diffuseColor = GetDiffuseMaterial(texCoord).rgb;
    vec3 normalWS = GetNormals(vs_in.fragNormal, vs_in.fragTangent, texCoord);
    vec3 viewDir = GetViewDir(vs_in.fragPositionWS);

    // colored lighting
    vec3 lighting = calculateLighting(
        material,
        texCoord,
        normalWS, 
        viewDir,
        vs_in.fragPositionWS);
    vec3 col = lighting * diffuseColor;
    float alpha = 1.0;
    
    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    finalColor = vec4(col, alpha);

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    finalColor = pow(finalColor, vec4(1.0/2.2));




    //vec3 lightDir = normalize(-sunlight.direction);
    //float NdotL = max(dot(normalWS, lightDir), 0.0);
    //finalColor = vec4(vec3(NdotL), 1.0);
    //finalColor = vec4(GetSpecularMaterial(texCoord).rgb, 1.0);

    //finalColor = vec4(diffuseColor, 1.0);
    //finalColor = vec4(GetSpecularMaterial(texCoord).rgb, 1.0);
    //finalColor = vec4(vec3(vs_in.fragNormal), 1.0);
    //finalColor = vec4(vec3(normalWS), 1.0);
}
