//#include "pch.h"
#include "model.h"

#include "camera.h"
#include "math/tiny_math.h"
#include "tiny_log.h"
#include "tiny_profiler.h"
#include "render/tiny_material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/version.h>

static const char* textureTypeToMatKey[] = {
    "none_NOKEY",
    "$clr.diffuse",
    "$clr.specular",
    "$clr.ambient",
    "$clr.emissive",
    "height_NOKEY",
    "normals_NOKEY",
    "$mat.shininess",
    "$mat.opacity",
    "displacement_NOKEY",
    "lightmap_NOKEY",
    "$mat.reflectivity",
    "unk_NOKEY"
};

struct AssimpMaterialKey
{
    const char *pKey = 0; unsigned int type = 0; unsigned int idx = 0;
    AssimpMaterialKey() = default;
    AssimpMaterialKey(const char *pKey, unsigned int type, unsigned int idx) :
    pKey(pKey), type(type), idx(idx) {}
};

MaterialProp GetMaterialFromType(aiMaterial* material, AssimpMaterialKey texture, AssimpMaterialKey fallbackColor, const char* meshMaterialDir) {
    PROFILE_FUNCTION();
    MaterialProp ret = MaterialProp();
    //TINY_ASSERT(material->GetTextureCount(type) <= 1); // there shouldn't... (???) be more than 1 of a particular texture type in a material (I.E. cant have 2 diffuse textures)
    aiString str;
    aiReturn hasTexture = material->Get(texture.pKey, texture.type, texture.idx, str);
    if (hasTexture == aiReturn::aiReturn_SUCCESS) 
    {
        std::string texturePath = meshMaterialDir;
        texturePath.append(str.C_Str());
        Texture tex = LoadTextureAsync(texturePath.c_str(), TextureProperties::None(), {}, true); // flip vert for opengl
        ret = MaterialProp(tex);
    }
    if (fallbackColor.pKey)
    {
        aiColor4D col;
        aiReturn hasColor = material->Get(fallbackColor.pKey, fallbackColor.type, fallbackColor.idx, col);
        if (hasColor == aiReturn_SUCCESS) 
        {
            ret.color = glm::vec4(col.r, col.g, col.b, col.a);
        }
    }
    return ret;
}

Material aiMaterialConvert(aiMaterial** materials, u32 meshMaterialIndex, const char* meshMaterialDir) {
    PROFILE_FUNCTION();
    if (DoesMaterialIdExist(meshMaterialIndex))
    {
        return Material(meshMaterialIndex);
    }
    aiMaterial* material = materials[meshMaterialIndex];
    aiString str;
    aiReturn hasName = material->Get(AI_MATKEY_NAME, str);
    if (hasName != aiReturn_SUCCESS) 
    {
        str = "UnnamedMaterial";
    }
    Material ret = NewMaterial(str.C_Str(), meshMaterialIndex);
    MaterialProp diffuse = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_DIFFUSE(0)), AssimpMaterialKey(AI_MATKEY_COLOR_DIFFUSE), meshMaterialDir);
    OverwriteMaterialProperty(ret, diffuse, DIFFUSE);
    MaterialProp ambient = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_AMBIENT(0)), AssimpMaterialKey(AI_MATKEY_COLOR_AMBIENT), meshMaterialDir);
    OverwriteMaterialProperty(ret, ambient, AMBIENT);
    MaterialProp specular = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_SPECULAR(0)), AssimpMaterialKey(AI_MATKEY_COLOR_SPECULAR), meshMaterialDir);
    OverwriteMaterialProperty(ret, specular, SPECULAR);
    MaterialProp emissive = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_EMISSIVE(0)), AssimpMaterialKey(AI_MATKEY_COLOR_EMISSIVE), meshMaterialDir);
    OverwriteMaterialProperty(ret, emissive, EMISSION);
    //MaterialProp height = GetMaterialFromType(material, aiTextureType_HEIGHT, meshMaterialDir);
    MaterialProp normal = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_NORMALS(0)), AssimpMaterialKey(), meshMaterialDir);
    if (!normal.hasTexture)
    {
        // the material api is kind of vague in some cases - for obj's, normal maps are often loaded into the heightmap slot.
        MaterialProp normalsFromHeightmap = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_HEIGHT(0)), AssimpMaterialKey(), meshMaterialDir);
        if (normalsFromHeightmap.hasTexture)
        {
            normal = normalsFromHeightmap;
        }
    }
    OverwriteMaterialProperty(ret, normal, NORMAL);
    // Usually there is a conversion function defined to map the linear color values in the texture to a suitable exponent
    MaterialProp shininess = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_SHININESS(0)), AssimpMaterialKey(AI_MATKEY_SHININESS), meshMaterialDir);
    OverwriteMaterialProperty(ret, shininess, SHININESS);
    MaterialProp opacity = GetMaterialFromType(material, AssimpMaterialKey(AI_MATKEY_TEXTURE_OPACITY(0)), AssimpMaterialKey(AI_MATKEY_COLOR_TRANSPARENT), meshMaterialDir);
    OverwriteMaterialProperty(ret, opacity, OPACITY);
    //MaterialProp displacement = GetMaterialFromType(material, aiTextureType_DISPLACEMENT, meshMaterialDir);
    //MaterialProp lightmap = GetMaterialFromType(material, aiTextureType_LIGHTMAP, meshMaterialDir);
    
    // if we have a specular coefficient, they are typically in the range [0, 1000]
    // we remap that to something that looks reasonable here
    if (!shininess.hasTexture)
    {
        // TODO: make configurable
        shininess.color.r = Math::Remap(shininess.color.r, 0.0, 1000.0, 0.0, 10.0);
    }
    
    return ret;
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, const char* meshMaterialDir) {
    PROFILE_FUNCTION();
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    std::vector<u32> indices;

    for (u32 i = 0; i < mesh->mNumVertices; i++) 
    {
        Vertex vertex;
        // process vertex attributes
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        if (mesh->mNormals) 
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        if (mesh->mTangents)
        {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        }

        if (mesh->mColors[0]) 
        {
            vector.x = mesh->mColors[0][i].r;
            vector.y = mesh->mColors[0][i].g;
            vector.z = mesh->mColors[0][i].b;
            vertex.color = vector;
        }

        if (mesh->mTextureCoords[0]) 
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else vertex.texCoords = glm::vec2(0.0f, 0.0f);

        // we -1 here because assimp by default always puts their "DefaultMaterial" as idx 0
        // and since we don't send that material to our shader, we need to pretend it doesn't
        // exist
        //vertex.materialId = mesh->mMaterialIndex - 1;

        vertices.push_back(vertex);
    }
    // process indices
    for (u32 i = 0; i < mesh->mNumFaces; i++) 
    {
        aiFace face = mesh->mFaces[i];
        for (u32 j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process material... assimp splits meshes with more than 1 material into multiple meshes so a mesh will always have 1 material
    Material meshMat = aiMaterialConvert(scene->mMaterials, mesh->mMaterialIndex, meshMaterialDir);

    Mesh m = Mesh(vertices, indices, meshMat);
    return m;
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const char* meshMaterialDir) {
    PROFILE_FUNCTION();
    for (u32 i = 0; i < node->mNumMeshes; i++) 
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh converted = processMesh(mesh, scene, meshMaterialDir);
        aiString meshName = node->mName;
        converted.name = meshName.C_Str();
        meshes.push_back(converted);
    }
    // process children
    for (u32 i = 0; i < node->mNumChildren; i++) 
    {
        processNode(node->mChildren[i], scene, meshes, meshMaterialDir);
    }
}

// sort by material
struct MeshCompare 
{
    bool operator() (const Mesh& mesh1, const Mesh& mesh2) const 
    {
        return mesh1.material.id < mesh2.material.id;
    }
};

Model::Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir) 
{
    PROFILE_FUNCTION();
    cachedShader = shader;
    Assimp::Importer import;
    const aiScene* scene = nullptr;
    { PROFILE_SCOPE("Assimp mesh read");
        scene = import.ReadFile(meshObjFile, 
            aiProcess_Triangulate | 
            aiProcess_OptimizeMeshes | 
            aiProcess_RemoveRedundantMaterials | 
            aiProcess_SplitLargeMeshes | 
            aiProcess_CalcTangentSpace |
            aiProcess_GenSmoothNormals);
    }
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        LOG_ERROR("[ASSIMP] Error: %s", import.GetErrorString());
        return;
    }
    processNode(scene->mRootNode, scene, meshes, meshMaterialDir);
    std::sort(meshes.begin(), meshes.end(), MeshCompare()); // when I implement a real renderer, meshes will be inserted in sorted order
    cachedBoundingBox = CalculateBoundingBox();
}
Model::Model(const Shader& shader, const std::vector<Mesh>& meshes) 
{
    cachedShader = shader;
    this->meshes = meshes;
    cachedBoundingBox = CalculateBoundingBox();
}


BoundingBox Model::CalculateBoundingBox() 
{
    PROFILE_FUNCTION();
    BoundingBox bounds = {};

    if (!this->meshes.empty()) 
    {
        glm::vec3 temp = glm::vec3(0);
        bounds = this->meshes[0].cachedBoundingBox;

        for (s32 i = 1; i < this->meshes.size(); i++) 
        {
            BoundingBox tempBounds = this->meshes[i].cachedBoundingBox;

            // get min for each component
            temp.x = (bounds.min.x < tempBounds.min.x) ? bounds.min.x : tempBounds.min.x;
            temp.y = (bounds.min.y < tempBounds.min.y) ? bounds.min.y : tempBounds.min.y;
            temp.z = (bounds.min.z < tempBounds.min.z) ? bounds.min.z : tempBounds.min.z;
            bounds.min = temp;

            // get max for each component
            temp.x = (bounds.max.x > tempBounds.max.x) ? bounds.max.x : tempBounds.max.x;
            temp.y = (bounds.max.y > tempBounds.max.y) ? bounds.max.y : tempBounds.max.y;
            temp.z = (bounds.max.z > tempBounds.max.z) ? bounds.max.z : tempBounds.max.z;
            bounds.max = temp;
        }
    }

    return bounds;
}


glm::mat4 GetMVP(const Transform& tf) 
{
    Camera& cam = Camera::GetMainCamera();
    // identity matrix to start out with
    glm::mat4 model = tf.ToModelMatrix();
    glm::mat4 view = cam.GetViewMatrix();
    glm::mat4 projection = cam.GetProjectionMatrix();
    return projection * view * model;
}

void DrawWithMaterials(const Shader& shader, const std::vector<Mesh>& meshes)
{
    PROFILE_FUNCTION();
    Material currentMat = {};
    for (const Mesh& mesh : meshes) 
    {
        // assuming meshes are sorted by material id, only change material uniforms when we switch materials
        if (currentMat.id != mesh.material.id)
        {
            mesh.material.SetShaderUniforms(shader);
            shader.use();
            currentMat = mesh.material;
        }
        mesh.Draw();
    }
}

void Model::Draw(const Shader& shader, const Transform& tf) const 
{
    PROFILE_FUNCTION();
    if (!isValid()) return;
    Camera& cam = Camera::GetMainCamera();
    glm::mat4 model = tf.ToModelMatrix();
    glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(model)));
    shader.setUniform("modelMat", model);
    shader.setUniform("normalMat", matNormal);
    SetLightingUniforms(shader);
    DrawWithMaterials(shader, meshes);
}

void Model::DrawMinimal() const
{
    PROFILE_FUNCTION();
    for (const Mesh& mesh : meshes) 
    {
        mesh.Draw();
    }
}

Mesh* Model::GetMesh(const std::string& name) 
{
    for (Mesh& mesh : meshes) 
    {
        if (mesh.name == name) return &mesh;
    }
    return nullptr;
}