#ifndef TINY_MATERIAL_H
#define TINY_MATERIAL_H


#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "render/texture.h"

enum TextureMaterialType {
    DIFFUSE = 0,
    AMBIENT,
    SPECULAR,
    NORMAL,
    SHININESS,
    EMISSION,
    OPACITY,
    OTHER,

    NUM_MATERIAL_TYPES,
};

struct MaterialProp 
{
    MaterialProp() = default;
    TAPI MaterialProp(glm::vec4 col);
    TAPI MaterialProp(const Texture& tex);
    TAPI void Delete();
    glm::vec4 color = glm::vec4(1);
    Texture texture = {};
    bool hasTexture = false;
};
struct Shader;
struct Material 
{
    u32 id = U32_INVALID_ID;
    Material() = default;
    Material(u32 id) { this->id = id; }
    TAPI void SetShaderUniforms(const Shader& shader) const;
    bool operator==(const Material& p) const { return id == p.id; }
};

struct MaterialInternal
{
    MaterialProp properties[TextureMaterialType::NUM_MATERIAL_TYPES] = {};
    const char name[64] = "DefaultMat";

    MaterialInternal& MaterialInternal::operator=(const MaterialInternal &mat);
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
    MaterialMap materialRegistry = {};
};

struct Arena;
void InitializeMaterialSystem(Arena* arena);

Material NewMaterial(const char* name, s32 materialIndex = -1);

void DeleteMaterial(Material material);

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type);

bool DoesMaterialIdExist(u32 materialID);

#endif