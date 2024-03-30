
// should be included in globals.glsl and nowhere else

// in sync with MaterialProp::DataType enum
const uint MatPropDataTypeUNK = 0;
const uint MatPropDataTypeINT = 1;
const uint MatPropDataTypeFLOAT = 2;
const uint MatPropDataTypeVECTOR = 3;
const uint MatPropDataTypeTEXTURE = 4;

// in sync with TextureMaterialType enum
const int TexMatTypeDIFFUSE = 0;
const int TexMatTypeAMBIENT = 1;
const int TexMatTypeSPECULAR = 2;
const int TexMatTypeNORMALS = 3;
const int TexMatTypeSHININESS = 4;
const int TexMatTypeEMISSION = 5;
const int TexMatTypeOPACITY = 6;
const int TexMatTypeOTHER = 7;
const int NUM_MATERIAL_TYPES = 8;

struct MaterialProperty
{
    vec4 color;
    int datai;
    sampler2D tex;
    uint dataType;
};

struct MaterialPropertyPacked
{
    vec4 color; // x holds float data, full vec4 is color <- cannot have float data and color data at same time
    uvec4 dataAndType; // x holds int data, w holds data type
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
// in sync with tiny_material
#define MAX_NUM_MATERIAL_PROPERTIES 15
struct MaterialPacked
{
    MaterialPropertyPacked defaultProperties[NUM_MATERIAL_TYPES];
    MaterialPropertyPacked extraProperties[MAX_NUM_MATERIAL_PROPERTIES - NUM_MATERIAL_TYPES];
};
uniform sampler2D materialSamplers[MAX_NUM_OBJECTS];

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