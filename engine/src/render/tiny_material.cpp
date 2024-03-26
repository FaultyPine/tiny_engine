#include "tiny_material.h"

#include "render/shader.h"
#include "tiny_log.h"
#include "tiny_profiler.h"
#include "mem/tiny_arena.h"
#include "tiny_engine.h"


MaterialInternal& MaterialInternal::operator=(const MaterialInternal &mat)
{
    TMEMCPY(properties, mat.properties, sizeof(properties));
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
    Material newMaterial = {};
    u32 nameHash = HashBytes((u8*)name, strnlen(name, MATERIAL_INTERNAL_NAME_MAX_LEN));
    newMaterial.id = materialIndex != -1 ? nameHash+materialIndex : nameHash;
    MaterialInternal newMaterialInternal = {};
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
    dataVec = col;
    dataType = MaterialProp::DataType::VECTOR;
}
MaterialProp::MaterialProp(const Texture& tex) 
{
    dataTex = tex.id;
    dataType = MaterialProp::DataType::TEXTURE;
}
MaterialProp::MaterialProp(s32 datai)
{
    this->datai = datai;
    dataType = MaterialProp::DataType::INT;
}
MaterialProp::MaterialProp(f32 dataf)
{
    this->dataf = dataf;
    dataType = MaterialProp::DataType::FLOAT;
}
void MaterialProp::Delete()
{ 
    if (dataType == MaterialProp::DataType::TEXTURE)
    {
        Texture(dataTex).Delete(); 
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
    shader.setUniform(TextFormat("material.%s.dataType", matVarStr), prop.dataType);
    switch (prop.dataType)
    {
        case MaterialProp::DataType::FLOAT:
        {
            shader.setUniform(TextFormat("material.%s.color.r", matVarStr), prop.dataf);
        } break;
        case MaterialProp::DataType::INT:
        {
            shader.setUniform(TextFormat("material.%s.datai", matVarStr), prop.datai);
        } break;
        case MaterialProp::DataType::VECTOR:
        {
            shader.setUniform(TextFormat("material.%s.color", matVarStr), prop.dataVec);
        } break;
        case MaterialProp::DataType::TEXTURE:
        {
            shader.TryAddSampler(prop.dataTex, TextFormat("material.%s.tex", matVarStr));
        } break;
        case MaterialProp::DataType::UNK:
        {
            //LOG_WARN("Blank material used");
        } break;
        default:
        {
            LOG_WARN("Unrecognized material property data type! %i", prop.dataType);
        } break;
    }
}

void Material::SetShaderUniforms(const Shader& shader) const
{
    PROFILE_FUNCTION();
    MaterialInternal& matInternal = GetMaterialInternal(*this);
    MaterialProp* properties = matInternal.properties;
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



