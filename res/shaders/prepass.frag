 

#include "globals.glsl"
#include "material.glsl"

// Gbuffer - any object-specific data needed for rendering must be written out here
// globals has attachment 0 as fragColor
// 0 -> (normals, depth)
// 1 -> (albedo, specular coeff)
// 2 -> (obj id, material id)
// 3 -> (positionWS)
layout (location = 1) out vec4 albedoSpecular;
layout (location = 2) out uvec2 objAndMaterial;
layout (location = 3) out vec3 positionWS;


void main() 
{
    float depth = gl_FragCoord.z;
    vec2 texCoord = vs_in.fragTexCoord;
    if (GetOpacityMaterial(texCoord).r < 0.2)
    {
        discard;
    }
    vec3 normalWS = GetNormals(vs_in.fragNormal, vs_in.fragTangent, texCoord);
    vec4 normsAndDepth = vec4(normalWS, depth);
    fragColor = normsAndDepth; // 1st color attachment
    // TODO: maybe just add emissive color onto the diffuse color?
    vec3 diffuseColor = GetDiffuseMaterial(texCoord).rgb;
    float specularCoeff = GetShininessMaterial(texCoord).r;
    albedoSpecular = vec4(diffuseColor, specularCoeff);
    objAndMaterial = uvec2(GetObjectID(), GetMaterialID());
    positionWS = vs_in.fragPositionWS;
}