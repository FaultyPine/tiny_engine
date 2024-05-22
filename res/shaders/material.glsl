
#include "shader_defines.glsl"
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
    uint dataType;
    int datai;
    vec4 color; // float data can be in x
    sampler2D tex;
};

struct Material
{
    MaterialProperty properties[MAX_NUM_MATERIAL_PROPERTIES];
    uint id;
};

uniform Material material;

uint GetMaterialID()
{
    return material.id;
}

uint GetMaterialPropertyDataType(uint matIdx)
{
    return material.properties[matIdx].dataType;
}
//  TexMatTypeXXXX, MatPropDataTypeXXXXX
bool DoesMaterialUseType(uint matIdx, uint dataType)
{
    return GetMaterialPropertyDataType(matIdx) == dataType;
}

vec4 GetMaterialColor(uint matType, vec2 uv) 
{
    switch (GetMaterialPropertyDataType(matType))
    {
        case MatPropDataTypeVECTOR:
        {
            return material.properties[matType].color;
        } break;
        case MatPropDataTypeTEXTURE:
        {
            // must be written out manually b/c we can't do copies on structs with opaque types (sampler2D)
            return texture(material.properties[matType].tex, uv);
        } break;
        case MatPropDataTypeFLOAT:
        {
            return vec4(material.properties[matType].color.r);
        } break;
        case MatPropDataTypeINT:
        {
            return vec4(float(material.properties[matType].datai));
        } break;
        default:
        {
            return vec4(0);
        } break;
    }
}





vec4 GetDiffuseMaterial(vec2 uv) 
{
    return GetMaterialColor(TexMatTypeDIFFUSE, uv);
}
vec4 GetAmbientMaterial(vec2 uv) 
{
    return GetMaterialColor(TexMatTypeAMBIENT, uv);
}
vec4 GetSpecularMaterial(vec2 uv)
{
    return GetMaterialColor(TexMatTypeSPECULAR, uv);
}
vec4 GetNormalMaterial(vec2 uv) 
{
    return GetMaterialColor(TexMatTypeNORMALS, uv);
}
vec4 GetShininessMaterial(vec2 uv) 
{
    return GetMaterialColor(TexMatTypeSHININESS, uv);
}
vec4 GetEmissiveMaterial(vec2 uv) 
{
    return GetMaterialColor(TexMatTypeEMISSION, uv);
}
vec4 GetOpacityMaterial(vec2 uv)
{
    return GetMaterialColor(TexMatTypeOPACITY, uv);
}

vec3 GetNormals(vec3 fragNormalOS, vec3 fragTangentOS, vec2 fragTexCoord) 
{
    vec3 normal = vec3(0);
    bool shouldUseSampler = DoesMaterialUseType(TexMatTypeNORMALS, MatPropDataTypeTEXTURE);
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