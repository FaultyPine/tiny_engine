


const int MatPropDataTypeUNK = 0;
const int MatPropDataTypeINT = 1;
const int MatPropDataTypeFLOAT = 2;
const int MatPropDataTypeVECTOR = 3;
const int MatPropDataTypeTEXTURE = 4;

struct MaterialProperty 
{
    vec4 color;
    int datai;
    sampler2D tex;
    int dataType;
};

struct Material 
{
    MaterialProperty diffuseMat;
    // TODO: don't use ambient material, ambient should always just be a color. 
    // If you really want an ambient "texture", use diffuse
    MaterialProperty ambientMat;
    MaterialProperty specularMat;
    MaterialProperty normalMat;
    MaterialProperty shininessMat;
    MaterialProperty emissiveMat;
    MaterialProperty opacityMat;
};

uniform Material material;

bool DoesMaterialUseTexture(in MaterialProperty prop)
{
    return prop.dataType == MatPropDataTypeTEXTURE;
}

vec4 GetMaterialColor(MaterialProperty mat, vec2 uv) 
{
    switch (mat.dataType)
    {
        case MatPropDataTypeVECTOR:
        {
            return mat.color;
        } break;
        case MatPropDataTypeTEXTURE:
        {
            return texture(mat.tex, uv);
        } break;
        default:
        {
            return vec4(0);
        } break;
    }
}
vec4 GetDiffuseMaterial(vec2 uv) 
{
    return GetMaterialColor(material.diffuseMat, uv);
}
vec4 GetAmbientMaterial(vec2 uv) 
{
    return GetMaterialColor(material.ambientMat, uv);
}
vec4 GetSpecularMaterial(vec2 uv)
{
    return GetMaterialColor(material.specularMat, uv);
}
vec4 GetNormalMaterial(vec2 uv) 
{
    return GetMaterialColor(material.normalMat, uv);
}
vec4 GetShininessMaterial(vec2 uv) 
{
    return GetMaterialColor(material.shininessMat, uv);
}
vec4 GetEmissiveMaterial(vec2 uv) 
{
    return GetMaterialColor(material.emissiveMat, uv);
}
vec4 GetOpacityMaterial(vec2 uv)
{
    return GetMaterialColor(material.opacityMat, uv);
}

vec3 GetNormals(vec3 fragNormalOS, vec3 fragTangentOS, vec2 fragTexCoord) 
{
    vec3 normal = vec3(0);
    bool shouldUseSampler = DoesMaterialUseTexture(material.normalMat);
    if (shouldUseSampler)
    {
        // TODO: put TBN in vert shader
        vec3 reOrthoFragTangentOS = (fragTangentOS - dot(fragTangentOS, fragNormalOS) * fragNormalOS);
        vec3 bitangentOS = cross(fragNormalOS, fragTangentOS);
        mat3 TBN = mat3(reOrthoFragTangentOS, bitangentOS, fragNormalOS);
        vec3 localNormal = 2.0 * GetNormalMaterial(fragTexCoord).rgb - 1.0;
        normal = normalize(TBN * localNormal); // world space normal
    }
    else
    {
        normal = normalize(fragNormalOS);
    }
    return normal;
}