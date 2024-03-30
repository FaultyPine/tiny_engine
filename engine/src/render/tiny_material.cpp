#include "tiny_material.h"

#include "render/shader.h"
#include "tiny_log.h"
#include "tiny_profiler.h"
#include "mem/tiny_arena.h"
#include "tiny_engine.h"
#include "shader_buffer.h"


MaterialInternal& MaterialInternal::operator=(const MaterialInternal &mat)
{
    TMEMCPY(&properties, &mat.properties, sizeof(properties));
    TMEMCPY((void*)name, mat.name, MATERIAL_INTERNAL_NAME_MAX_LEN);
    return *this;
}

void InitializeMaterialSystem(Arena* arena)
{
    EngineContext& ctx = GetEngineCtx();
    TINY_ASSERT(ctx.textureCache && "Initialize the texture cache before the material system!");
    MaterialRegistry* reg = arena_alloc_and_init<MaterialRegistry>(arena);
    ctx.materialRegistry = reg;

    Material dummyMaterial = NewMaterial("DummyMaterial");
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

Material NewMaterial(const char* name, s32 materialIndex) {
    MaterialRegistry& matRegistry = GetMaterialRegistry();
    u32 nameSize = strnlen(name, MATERIAL_INTERNAL_NAME_MAX_LEN);
    u32 materialHash = HashBytes((u8*)name, nameSize);
    materialHash = materialIndex != -1 ? materialHash+materialIndex : materialHash;
    Material newMaterial = Material(materialHash);
    if (matRegistry.materialRegistry.count(newMaterial))
    {
        const MaterialInternal& alreadyExistingMat = matRegistry.materialRegistry[newMaterial];
        bool areNamesSame = strncmp(alreadyExistingMat.name, name, MATERIAL_INTERNAL_NAME_MAX_LEN) == 0;
        if (areNamesSame)
        {
            // if we try to create a material with the same name, just return the already existing one
            return newMaterial;
        }
        else
        {
            // two different names hashed the same
            TINY_ASSERT(false && "Material name hash collision!");
        }
    }
    // on gpu we use this gpu material index to index a list of materials.
    // if we have collisions with this thats an error state because we don't check hash collisions on gpu
    u32 gpuMaterialIdx = materialHash % MAX_NUM_MATERIALS;
    if (matRegistry.materialGPUIdxChecker.count(gpuMaterialIdx))
    {
        LOG_ERROR("Material gpu idx collision!");
    }
    else
    {
        matRegistry.materialGPUIdxChecker.insert(gpuMaterialIdx);
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
    for (u32 i = 0; i < ARRAY_SIZE(matInternal.properties.defaultProperties); i++)
    {
        matInternal.properties.defaultProperties[i].Delete();
    }
    for (u32 i = 0; i < ARRAY_SIZE(matInternal.properties.extraProperties); i++)
    {
        matInternal.properties.extraProperties[i].Delete();
    }
    GetMaterialRegistry().materialRegistry.erase(material);
}

void OverwriteMaterialProperty(Material material, const MaterialProp& prop, TextureMaterialType type)
{
    MaterialInternal& matInternal = GetMaterialInternal(material);
    matInternal.properties.defaultProperties[type] = prop;
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


const char* GetTexMatTypeString(TextureMaterialType type) {
    switch (type) 
    {
        case DIFFUSE: return "diffuseMat";
        case SPECULAR: return "specularMat";
        case AMBIENT: return "ambientMat";
        case NORMALS: return "normalMat";
        case SHININESS: return "shininessMat";
        case EMISSION: return "emissiveMat";
        case OPACITY: return "opacityMat";
        case OTHER: return "otherMat";
        default: return "unknownMat";
    }
}

static void SetShaderUniformForMatProp(const MaterialProp& prop, const Shader& shader, const char* matVarStr)
{
    shader.setUniform(TextFormat("material.%s.dataType", matVarStr), prop.GetDataType());
    switch (prop.GetDataType())
    {
        case MaterialProp::DataType::FLOAT:
        {
            shader.setUniform(TextFormat("material.%s.color.r", matVarStr), prop.FloatData());
        } break;
        case MaterialProp::DataType::INT:
        {
            shader.setUniform(TextFormat("material.%s.datai", matVarStr), prop.IntData());
        } break;
        case MaterialProp::DataType::VECTOR:
        {
            shader.setUniform(TextFormat("material.%s.color", matVarStr), prop.VecData());
        } break;
        case MaterialProp::DataType::TEXTURE:
        {
            shader.TryAddSampler(Texture(prop.TextureData()), TextFormat("material.%s.tex", matVarStr));
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
    MaterialProp* properties = matInternal.properties.defaultProperties;
    #define SET_MATERIAL_UNIFORMS(matType) \
    { \
        const MaterialProp& matVar = properties[matType]; \
        const char* matVarStr = GetTexMatTypeString(matType); \
        SetShaderUniformForMatProp(matVar, shader, matVarStr); \
    }
    SET_MATERIAL_UNIFORMS(DIFFUSE);
    SET_MATERIAL_UNIFORMS(AMBIENT);
    SET_MATERIAL_UNIFORMS(SPECULAR);
    SET_MATERIAL_UNIFORMS(NORMALS);
    SET_MATERIAL_UNIFORMS(SHININESS);
    SET_MATERIAL_UNIFORMS(EMISSION);
    SET_MATERIAL_UNIFORMS(OPACITY);
}



