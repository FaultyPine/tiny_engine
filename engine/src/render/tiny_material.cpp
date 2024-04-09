#include "tiny_material.h"

#include "render/shader.h"
#include "tiny_log.h"
#include "tiny_profiler.h"
#include "mem/tiny_arena.h"
#include "tiny_engine.h"
#include "shader_buffer.h"


size_t MaterialPropHasher::operator()(const MaterialProp& p) const
{
    // MaterialProp is (reasonably small) POD so just hash the whole thing
    return HashBytesL((u8*)&p, sizeof(MaterialProp));
}

Material& Material::operator=(const Material& p) 
{ 
    this->id = p.id; 
    TMEMSET((void*)dbgName, 0, 30); 
    TMEMCPY((void*)dbgName, p.dbgName, 30); 
    return *this;
}


MaterialInternal& MaterialInternal::operator=(const MaterialInternal &mat)
{
    TMEMCPY(&properties, &mat.properties, sizeof(properties));
    TMEMSET((void*)name, 0, MATERIAL_INTERNAL_NAME_MAX_LEN);
    TMEMCPY((void*)name, mat.name, MATERIAL_INTERNAL_NAME_MAX_LEN);
    return *this;
}

void InitializeMaterialSystem(Arena* arena)
{
    EngineContext& ctx = GetEngineCtx();
    TINY_ASSERT(ctx.textureCache && "Initialize the texture cache before the material system!");
    MaterialRegistry* reg = arena_alloc_and_init<MaterialRegistry>(arena);
    ctx.materialRegistry = reg;

    Material dummyMaterial = NewMaterial("DummyMaterial", U32_INVALID_ID);
    OverwriteMaterialProperty(dummyMaterial, MaterialProp(GetDummyTexture()), TextureMaterialType::DIFFUSE);
    ctx.materialRegistry->dummyMaterial = dummyMaterial;
}

MaterialRegistry& GetMaterialRegistry()
{
    return *GetEngineCtx().materialRegistry;
}

MaterialInternal& GetMaterialInternal(Material material)
{
    return GetMaterialRegistry().materialRegistry[material];
}

Material GetDummyMaterial()
{
    return GetMaterialRegistry().dummyMaterial;
}

bool DoesMaterialIdExist(u32 materialID)
{
    return GetMaterialRegistry().materialRegistry.count(Material(materialID)) > 0;
}

Material NewMaterial(const char* name, u32 materialHash) {
    u32 nameSize = strnlen(name, MATERIAL_INTERNAL_NAME_MAX_LEN);
    Material newMaterial = Material(materialHash);
    TMEMSET((void*)newMaterial.dbgName, 0, nameSize);
    TMEMCPY((void*)newMaterial.dbgName, name, nameSize);
    MaterialRegistry& matRegistry = GetMaterialRegistry();
    if (matRegistry.materialRegistry.count(newMaterial))
    {
        const MaterialInternal& alreadyExistingMat = matRegistry.materialRegistry[newMaterial];
        bool areNamesSame = strncmp(alreadyExistingMat.name, name, nameSize) == 0;
        if (areNamesSame)
        {
            // if we try to create a material with the same name, just return the already existing one
            return newMaterial;
        }
        else
        {
            // two different names hashed the same
            LOG_ERROR("Material name hash collision! %s and %s = %u", alreadyExistingMat.name, name, newMaterial.id);
            TINY_ASSERT(false);
        }
    }
    MaterialInternal newMaterialInternal = {};
    TMEMSET((void*)&newMaterialInternal.name[0], 0, MATERIAL_INTERNAL_NAME_MAX_LEN);
    TMEMCPY((void*)&newMaterialInternal.name[0], name, nameSize);
    matRegistry.materialRegistry[newMaterial] = newMaterialInternal;
    return newMaterial;
}

void DeleteMaterial(Material material)
{
    MaterialInternal& matInternal = GetMaterialInternal(material);
    for (u32 i = 0; i < ARRAY_SIZE(matInternal.properties); i++)
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
    GetDataType() = MaterialProp::DataType::VECTOR;
    VecData() = col;
}
MaterialProp::MaterialProp(const Texture& tex) 
{
    GetDataType() = MaterialProp::DataType::TEXTURE;
    TextureData() = tex.id;
}
MaterialProp::MaterialProp(u32 datai)
{
    GetDataType() = MaterialProp::DataType::INT;
    IntData() = datai;
}
MaterialProp::MaterialProp(f32 dataf)
{
    GetDataType() = MaterialProp::DataType::FLOAT;
    FloatData() = dataf;
}
void MaterialProp::Delete()
{ 
    if (GetDataType() == MaterialProp::DataType::TEXTURE)
    {
        Texture(TextureData()).Delete(); 
    }
}


static void SetShaderUniformForMatProp(
    u32 matIdx, // equivalent to TextureMaterialType
    const MaterialProp& prop, 
    const Shader& shader)
{
    shader.setUniform(TextFormat("material.properties[%i].dataType", matIdx), prop.GetDataType());
    switch (prop.GetDataType())
    {
        case MaterialProp::DataType::FLOAT:
        {
            shader.setUniform(TextFormat("material.properties[%i].color.r", matIdx), prop.FloatData());
        } break;
        case MaterialProp::DataType::INT:
        {
            shader.setUniform(TextFormat("material.properties[%i].datai", matIdx), prop.IntData());
        } break;
        case MaterialProp::DataType::VECTOR:
        {
            shader.setUniform(TextFormat("material.properties[%i].color", matIdx), prop.VecData());
        } break;
        case MaterialProp::DataType::TEXTURE:
        {
            shader.TryAddSampler(Texture(prop.TextureData()), TextFormat("material.properties[%i].tex", matIdx));
        } break;
        case MaterialProp::DataType::UNK:
        {
            //LOG_WARN("Blank material used");
        } break;
        default:
        {
            LOG_WARN("Unrecognized material property data type! %i", prop.GetDataType());
        } break;
    }
}

void Material::SetShaderUniforms(const Shader& shader) const
{
    PROFILE_FUNCTION();
    MaterialInternal& matInternal = GetMaterialInternal(*this);
    MaterialProp* properties = &matInternal.properties[0];
    for (u32 i = 0; i < NUM_MATERIAL_TYPES; i++)
    {
        const MaterialProp& matVar = properties[i];
        SetShaderUniformForMatProp(i, matVar, shader);
    }
}



