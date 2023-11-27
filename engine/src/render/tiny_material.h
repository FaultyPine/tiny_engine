#ifndef TINY_MATERIAL_H
#define TINY_MATERIAL_H


#include "tiny_defines.h"
#include "math/tiny_math.h"
#include "render/texture.h"

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
    u32 id;
    Material() = default;
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

Material NewMaterial(const char* name);

void DeleteMaterial(Material material);

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type);


#endif