 
#define NO_VS_OUT
#include "globals.glsl"
#undef NO_VS_OUT
#include "lighting.glsl"

uniform mat4 spriteModelMat;
uniform mat4 spriteViewMat;
uniform mat4 spriteProjectionMat;
uniform int shouldFlipY;
in vec2 fragTexCoord;

void main() 
{
    float alpha = 1.0;
    vec2 texCoord = fragTexCoord;
    float uvY = (shouldFlipY*(1-texCoord.y)) + ((1-shouldFlipY)*(texCoord.y));
    texCoord = vec2(texCoord.x, uvY);
    // base diffuse material texture
    vec4 albedoSpec = GetAlbedoSpec(texCoord);
    vec3 diffuseMaterial = albedoSpec.rgb;
    float shininess = albedoSpec.a;
    vec4 depthNormals = GetDepthNormals(texCoord);
    vec3 normalWS = depthNormals.rgb;
    float depth = depthNormals.a;
    // reconstruct ws position from depth
    vec3 fragPositionWS = GetPositionWS(texCoord);
    vec3 viewDir = GetViewDir(fragPositionWS);
    //uvec2 objIdMat = GetObjMatId(texCoord);
    //uint objID = objIdMat.r;
    //uint matId = objIdMat.g;

    // colored lighting
    vec3 lighting = calculateLighting(
        texCoord,
        normalWS, 
        viewDir,
        fragPositionWS,
        diffuseMaterial,
        shininess);
    vec3 col = lighting * diffuseMaterial;
    
    // lighting includes diffuse, specular, and ambient light along with base diffuse color
    fragColor = vec4(vec3(col), alpha);
}
