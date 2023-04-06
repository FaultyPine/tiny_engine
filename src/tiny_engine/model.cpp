#include "pch.h"
#include "model.h"

#include "ObjParser.h"
#include "camera.h"
#include "tiny_engine/math.h"

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

MaterialProp GetMaterialFromType(aiMaterial* material, aiTextureType type, const char* meshMaterialDir) {
    MaterialProp ret;
    ASSERT(material->GetTextureCount(type) == 1); // there shouldn't... (???) be more than 1 of a particular texture type in a material (I.E. cant have 2 diffuse textures)
    aiString str;
    aiReturn hasTexture = material->GetTexture(type, 0, &str);
    if (hasTexture != aiReturn::aiReturn_SUCCESS) {
        aiColor4D col;
        const char* matkey = textureTypeToMatKey[type];
        aiReturn hasColor = material->Get(matkey, 0, 0, col);
        if (hasColor == aiReturn_SUCCESS) {
            ret.hasTexture = false;
            ret.color = glm::vec4(col.r, col.g, col.b, col.a);
        }
    }
    else {
        std::string texturePath = meshMaterialDir;
        texturePath.append(str.C_Str());
        Texture tex = LoadTexture(texturePath, TextureProperties::None(), true); // flip vert for opengl
        tex.texpath = texturePath;
        ret.hasTexture = true;
        ret.texture = tex;
    }
    return ret;
}

Material aiMaterialConvert(aiMaterial* material, const char* meshMaterialDir) {
    // TODO: since assimp only has shininess as a texture, might want to change my material impl to use
    // a texture for shininess too... since thats usually how it's done
    float shininess = 0.0f;
    aiString str;
    std::string name = "";
    aiReturn hasName = material->Get(AI_MATKEY_NAME, str);
    if (hasName == aiReturn_SUCCESS) {
        name = std::string(str.C_Str());
    }

    MaterialProp diffuse = GetMaterialFromType(material, aiTextureType_DIFFUSE, meshMaterialDir);
    MaterialProp ambient = GetMaterialFromType(material, aiTextureType_AMBIENT, meshMaterialDir);
    MaterialProp specular = GetMaterialFromType(material, aiTextureType_SPECULAR, meshMaterialDir);
    MaterialProp normal = GetMaterialFromType(material, aiTextureType_NORMALS, meshMaterialDir);

    Material ret = Material(diffuse, ambient, specular, normal, shininess, name);
    return ret;
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, const char* meshMaterialDir) {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Material> materials;

    for (u32 i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        if (mesh->mNormals) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        if (mesh->mColors[0]) {
            vector.x = mesh->mColors[0][i].r;
            vector.y = mesh->mColors[0][i].g;
            vector.z = mesh->mColors[0][i].b;
            vertex.color = vector;
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else vertex.texCoords = glm::vec2(0.0f, 0.0f);

        // we -1 here because assimp by default puts their "DefaultMaterial" as idx 0
        // and since we don't send that material to our shader, we need to pretend it doesn't
        // exist
        vertex.materialId = mesh->mMaterialIndex - 1;

        vertices.push_back(vertex);
    }
    // process indices
    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (u32 j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process material
    /*
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        Material m = aiMaterialConvert(material, meshMaterialDir);
        materials.push_back(m);
    }
    */

    for (u32 i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiString name;
        aiReturn hasName = material->Get(AI_MATKEY_NAME, name);
        if (hasName == aiReturn_SUCCESS) {
            //std::cout << "mat name = " << name.C_Str() << "\n";
            if (std::string(name.C_Str()) != "DefaultMaterial") {
                Material m = aiMaterialConvert(material, meshMaterialDir);
                materials.push_back(m);
            }
        }
    }

    Mesh m = Mesh(vertices, indices, materials);
    return m;
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const char* meshMaterialDir) {
    // process all the node's meshes (if any)
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh converted = processMesh(mesh, scene, meshMaterialDir);
        aiString meshName = node->mName;
        converted.name = meshName.C_Str();
        meshes.push_back(converted);
    }
    // then do the same for each of its children
    for (u32 i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes, meshMaterialDir);
    }
}

Model::Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir) {
#if 1
    Assimp::Importer import;
    //std::string extList = "";
    //import.GetExtensionList(extList);
    //std::cout << extList << "\n";
    //std::cout << aiGetVersionMajor() << "." << aiGetVersionMinor() << "." << aiGetVersionRevision() << "\n";
    const aiScene* scene = import.ReadFile(meshObjFile, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "[assimp] Error: " << import.GetErrorString() << "\n";
        return;
    }
    processNode(scene->mRootNode, scene, meshes, meshMaterialDir);
#else
    meshes = LoadObjMesh(meshObjFile, meshMaterialDir);
#endif
    cachedShader = shader;
}
Model::Model(const Shader& shader, const std::vector<Mesh>& meshes) {
    cachedShader = shader;
    this->meshes = meshes;
}


BoundingBox Model::GetBoundingBox() {
    BoundingBox bounds = {};

    if (!this->meshes.empty()) {
        glm::vec3 temp = glm::vec3(0);
        bounds = this->meshes[0].GetMeshBoundingBox();

        for (s32 i = 1; i < this->meshes.size(); i++) {
            BoundingBox tempBounds = this->meshes[i].GetMeshBoundingBox();

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

bool AreActiveLightsInFront(const std::vector<Light>& lights) {
    for (u32 i = 0; i < lights.size(); i++) {
        if (!lights.at(i).enabled) {
            for (u32 j = i+1; j < lights.size(); j++) {
                if (lights.at(i).enabled) {
                    return false;
                }
            }
        }
    }
    return true;
}

void Model::Draw(const Shader& shader, const Transform& tf, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(tf.position, tf.scale, tf.rotation, tf.rotationAxis);
        shader.setUniform("modelMat", model);
        glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(model)));
        shader.setUniform("normalMat", matNormal);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, tf);
    }
}
void Model::Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        shader.setUniform("modelMat", modelMat);
        glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(modelMat)));
        shader.setUniform("normalMat", matNormal);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, mvp);
    }
}
void Model::DrawMinimal(const Shader& shader) const {
    for (const Mesh& mesh : meshes) {
        mesh.DrawMinimal(shader);
    }
}

void Model::DrawInstanced(const Shader& shader, u32 numInstances, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        Camera& cam = Camera::GetMainCamera();
        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 projection = cam.GetProjectionMatrix();

        shader.setUniform("viewMat", view);
        shader.setUniform("projectionMat", projection);
        shader.setUniform("viewPos", cam.cameraPos);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.DrawInstanced(shader, numInstances);
    }
}

Mesh* Model::GetMesh(const std::string& name) {
    for (Mesh& mesh : meshes) {
        if (mesh.name == name) return &mesh;
    }
    return nullptr;
}