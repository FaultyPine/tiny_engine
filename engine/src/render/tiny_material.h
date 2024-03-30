#ifndef TINY_MATERIAL_H
#define TINY_MATERIAL_H


#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "render/texture.h"
#include "tiny_log.h"

#include <set>

enum TextureMaterialType {
    DIFFUSE = 0,
    AMBIENT,
    SPECULAR,
    NORMALS,
    SHININESS,
    EMISSION,
    OPACITY,
    OTHER,

    NUM_MATERIAL_TYPES,
};
struct Shader;
struct Material 
{
    u32 id = U32_INVALID_ID;
    Material() = default;
    Material(u32 id) { this->id = id; }
    TAPI bool isValid() const { return id == U32_INVALID_ID; }
    TAPI void SetShaderUniforms(const Shader& shader) const;
    bool operator==(const Material& p) const { return id == p.id; }
};

// these are intentionally meant to be 16 byte aligned (2 vec4s) so they can be used in std140 gpu buffers
struct MaterialProp 
{
    enum DataType : u32
    {
        UNK,
        INT,
        FLOAT, 
        VECTOR,
        TEXTURE,
    };
    MaterialProp() = default;
    TAPI MaterialProp(glm::vec4 col);
    TAPI MaterialProp(const Texture& tex);
    TAPI MaterialProp(u32 datai);
    TAPI MaterialProp(f32 dataf);
    TAPI void Delete();
    
    glm::vec4& VecData() { TINY_ASSERT(GetDataType()==VECTOR); return vec; }
    f32& FloatData() { TINY_ASSERT(GetDataType()==FLOAT); return vec.x; }
    u32& IntData() { TINY_ASSERT(GetDataType()==INT); return dataAndType.x; }
    u32& TextureData() { TINY_ASSERT(GetDataType()==TEXTURE); return dataAndType.x; }
    const glm::vec4& VecData() const { TINY_ASSERT(GetDataType()==VECTOR); return vec; }
    const f32& FloatData() const { TINY_ASSERT(GetDataType()==FLOAT); return vec.x; }
    const u32& IntData() const { TINY_ASSERT(GetDataType()==INT); return dataAndType.x; }
    const u32& TextureData() const { TINY_ASSERT(GetDataType()==TEXTURE); return dataAndType.x; }
    DataType& GetDataType() { return *(DataType*)&dataAndType.w; }
    const DataType& GetDataType() const { return (DataType)dataAndType.w; }

    glm::vec4 vec; // float data can be stored in x, color data in full vec4
    glm::uvec4 dataAndType; // int OR texture data in x, DataType stored in w
};

// max properties on any material - including ones for material types (albedo, norms, emission, specular, roughness, etc)
#define MAX_NUM_MATERIAL_PROPERTIES 15
struct MaterialPropertiesPacked
{
    MaterialProp defaultProperties[TextureMaterialType::NUM_MATERIAL_TYPES];
    MaterialProp extraProperties[MAX_NUM_MATERIAL_PROPERTIES - TextureMaterialType::NUM_MATERIAL_TYPES];
};
 
struct MaterialInternal
{
    MaterialPropertiesPacked properties;
    #define MATERIAL_INTERNAL_NAME_MAX_LEN 64
    const char name[MATERIAL_INTERNAL_NAME_MAX_LEN] = "DefaultMat";

    MaterialInternal& operator=(const MaterialInternal &mat);
};

struct MaterialHasher {
    size_t operator()(const Material& p) const
    {
        return p.id;
    }
};

typedef std::unordered_map<Material, MaterialInternal, MaterialHasher> MaterialMap;

struct MaterialRegistry 
{
    u32 currentMaterialId = 0;
    Material dummyMaterial = {};
    MaterialMap materialRegistry = {};
    std::set<u32> materialGPUIdxChecker = {};
};

struct Arena;
void InitializeMaterialSystem(Arena* arena);

Material NewMaterial(const char* name, s32 materialIndex = -1);

void DeleteMaterial(Material material);

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type);

MaterialInternal& GetMaterialInternal(Material material);

bool DoesMaterialIdExist(u32 materialID);

Material GetDummyMaterial();
#endif