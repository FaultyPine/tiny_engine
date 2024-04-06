#ifndef TINY_MATERIAL_H
#define TINY_MATERIAL_H


#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "render/texture.h"
#include "tiny_log.h"

#include <set>

#include "res/shaders/shader_defines.glsl"

enum TextureMaterialType : u32 {
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
    const char dbgName[30] = "NoName";
    u32 id = U32_INVALID_ID;
    Material() = default;
    Material(u32 id) { this->id = id; }
    TAPI bool isValid() const { return id != U32_INVALID_ID; }
    TAPI void SetShaderUniforms(const Shader& shader) const;
    bool operator==(const Material& p) const { return id == p.id; }
    Material& operator=(const Material& p);
};

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
    
    glm::vec4& VecData() { TINY_ASSERT(GetDataType()==VECTOR); return dataVec; }
    f32& FloatData() { TINY_ASSERT(GetDataType()==FLOAT); return dataf; }
    u32& IntData() { TINY_ASSERT(GetDataType()==INT); return datai; }
    u32& TextureData() { TINY_ASSERT(GetDataType()==TEXTURE); return dataTex; }
    const glm::vec4& VecData() const { TINY_ASSERT(GetDataType()==VECTOR); return dataVec; }
    const f32& FloatData() const { TINY_ASSERT(GetDataType()==FLOAT); return dataf; }
    const u32& IntData() const { TINY_ASSERT(GetDataType()==INT); return datai; }
    const u32& TextureData() const { TINY_ASSERT(GetDataType()==TEXTURE); return dataTex; }
    DataType& GetDataType() { return dataType; }
    const DataType& GetDataType() const { return (DataType)dataType; }

    DataType dataType = UNK;
    union
    {
        u32 datai;
        f32 dataf;
        glm::vec4 dataVec;
        u32 dataTex; // texture id. B/c Texture has nontrivial ctor, can't use it in union. do Texture(dataTex) to access texture funcs
    };
};

struct MaterialPropHasher
{
    size_t operator()(const MaterialProp& p) const;
};
 
struct MaterialInternal
{
    MaterialProp properties[MAX_NUM_MATERIAL_PROPERTIES];
    static_assert(MAX_NUM_MATERIAL_PROPERTIES >= TextureMaterialType::NUM_MATERIAL_TYPES);
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
};

struct Arena;
void InitializeMaterialSystem(Arena* arena);

Material NewMaterial(const char* name, u32 materialHash);

void DeleteMaterial(Material material);

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type);

MaterialInternal& GetMaterialInternal(Material material);

bool DoesMaterialIdExist(u32 materialID);

Material GetDummyMaterial();
#endif