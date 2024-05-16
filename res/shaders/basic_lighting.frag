 

#include "globals.glsl"
#include "lighting.glsl"

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
        texCoord,
        normalWS, 
        viewDir,
        vs_in.fragPositionWS);
    vec3 col = lighting * diffuseColor;
    float alpha = 1.0;
    
    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    fragColor = vec4(col, alpha);
}
