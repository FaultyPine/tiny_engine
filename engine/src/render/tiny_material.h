#ifndef TINY_MATERIAL_H
#define TINY_MATERIAL_H


#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "render/texture.h"

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
struct MaterialProp 
{
    enum DataType : s32
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
    TAPI MaterialProp(s32 datai);
    TAPI MaterialProp(f32 dataf);
    TAPI void Delete();
    DataType dataType = UNK;
    union
    {
        s32 datai;
        f32 dataf;
        glm::vec4 dataVec;
        u32 dataTex; // texture id. B/c Texture has nontrivial ctor, can't use it in union. do Texture(dataTex) to access texture funcs
    };
};

struct MaterialInternal
{
    MaterialProp properties[TextureMaterialType::NUM_MATERIAL_TYPES];
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

Material NewMaterial(const char* name, s32 materialIndex = -1);

void DeleteMaterial(Material material);

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type);

MaterialInternal& GetMaterialInternal(Material material);

bool DoesMaterialIdExist(u32 materialID);

Material GetDummyMaterial();
#endif