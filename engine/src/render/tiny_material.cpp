#include "tiny_material.h"

#include "render/shader.h"
#include "tiny_log.h"
#include "tiny_profiler.h"
#include "tiny_alloc.h"
#include "tiny_engine.h"


MaterialInternal& MaterialInternal::operator=(const MaterialInternal &mat)
{
    TMEMCPY(properties, mat.properties, sizeof(properties));
    TMEMCPY(properties, mat.properties, sizeof(properties));
    return *this;
}

void InitializeMaterialSystem(Arena* arena)
{
    MaterialRegistry* reg = (MaterialRegistry*)arena_alloc(arena, sizeof(MaterialRegistry));
    new(&reg->materialRegistry) MaterialMap();
    GetEngineCtx().materialRegistry = reg;
}

MaterialRegistry& GetMaterialRegistry()
{
    return *GetEngineCtx().materialRegistry;
}

MaterialInternal& GetMaterialInternal(Material material)
{
    return GetMaterialRegistry().materialRegistry[material];
}

bool DoesMaterialIdExist(u32 materialID)
{
    return GetMaterialRegistry().materialRegistry.count(Material(materialID)) > 0;
}

Material NewMaterial(const char* name, s32 materialIndex) {
    MaterialRegistry& matRegistry = GetMaterialRegistry();
    Material newMaterial = {};
    newMaterial.id = materialIndex != -1 ? materialIndex : HashBytes((u8*)name, strnlen(name, 64));
    MaterialInternal newMaterialInternal;
    newMaterialInternal.properties[DIFFUSE] = {};
    newMaterialInternal.properties[AMBIENT] = {};
    newMaterialInternal.properties[SPECULAR] = {};
    newMaterialInternal.properties[NORMAL] = {};
    newMaterialInternal.properties[SHININESS] = {}; // :/ unify naming
    newMaterialInternal.properties[EMISSION] = {};
    newMaterialInternal.properties[OPACITY] = {};
    u32 materialMaxNameSize = ARRAY_SIZE(newMaterialInternal.name);
    TMEMCPY((void*)&newMaterialInternal.name[0], name, strnlen(name, materialMaxNameSize));
    matRegistry.materialRegistry[newMaterial] = newMaterialInternal;
    return newMaterial;
}

void DeleteMaterial(Material material)
{
    MaterialInternal& matInternal = GetMaterialInternal(material);
    for (u32 i = 0; i < TextureMaterialType::NUM_MATERIAL_TYPES; i++)
    {
        matInternal.properties[i].Delete();
    }
    GetMaterialRegistry().materialRegistry.erase(material);
}

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type)
{
    MaterialInternal& matInternal = GetMaterialInternal(material);
    matInternal.properties[type] = prop;
}

MaterialProp::MaterialProp(glm::vec4 col) 
{
    color = col;
    hasTexture = false;
}
MaterialProp::MaterialProp(const Texture& tex) 
{
    texture = tex;
    hasTexture = true;
}
void MaterialProp::Delete()
{ 
    if (hasTexture)
    {
        texture.Delete(); 
    }
}


const char* GetTexMatTypeString(TextureMaterialType type) {
    switch (type) 
    {
        case DIFFUSE: return "diffuseMat";
        case SPECULAR: return "specularMat";
        case AMBIENT: return "ambientMat";
        case NORMAL: return "normalMat";
        case SHININESS: return "shininessMat";
        case EMISSION: return "emissiveMat";
        case OPACITY: return "opacityMat";
        case OTHER: return "otherMat";
        default: return "unknownMat";
    }
}



void Material::SetShaderUniforms(const Shader& shader) const
{
    PROFILE_FUNCTION();
    MaterialInternal& matInternal = GetMaterialRegistry().materialRegistry[*this];
    MaterialProp* properties = matInternal.properties;
    #define SET_MATERIAL_UNIFORMS(matType) \
    { \
        const MaterialProp& matVar = properties[matType]; \
        const char* matVarStr = GetTexMatTypeString(matType); \
        shader.setUniform(TextFormat("material.%s.useSampler", matVarStr), matVar.hasTexture); \
        if (matVar.hasTexture) { \
            shader.TryAddSampler(matVar.texture, TextFormat("material.%s.tex", matVarStr)); \
        } \
        shader.setUniform(TextFormat("material.%s.color", matVarStr), matVar.color); \
    }

    SET_MATERIAL_UNIFORMS(DIFFUSE);
    SET_MATERIAL_UNIFORMS(AMBIENT);
    SET_MATERIAL_UNIFORMS(SPECULAR);
    SET_MATERIAL_UNIFORMS(NORMAL);
    SET_MATERIAL_UNIFORMS(SHININESS);
    SET_MATERIAL_UNIFORMS(OPACITY);
    SET_MATERIAL_UNIFORMS(EMISSION);
}



