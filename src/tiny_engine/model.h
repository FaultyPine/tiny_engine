#ifndef TINY_MODEL_H
#define TINY_MODEL_H

//#include "pch.h"
#include "mesh.h"
#include "tiny_lights.h"

struct Model {
    Model() = default;
    Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir);
    Model(const Shader& shader, const std::vector<Mesh>& meshes);

    BoundingBox GetBoundingBox();

    // draw with transform
    void Draw(const Shader& shader, const Transform& tf, const std::vector<Light>& lights = {}) const;
    void Draw(const Transform& tf, const std::vector<Light>& lights = {}) const {
        Draw(cachedShader, tf, lights);
    }
    // draw with mvp matrix
    void Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights = {}) const;
    void Draw(const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights = {}) const {
        Draw(cachedShader, mvp, modelMat, lights);
    }

    void DrawMinimal(const Shader& shader) const;
    
    // instanced drawing
    void DrawInstanced(const Shader& shader, u32 numInstances, const std::vector<Light>& lights = {}) const;
    void DrawInstanced(u32 numInstances, const std::vector<Light>& lights = {}) const {
        DrawInstanced(cachedShader, numInstances, lights);
    }
    void EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents) {
        for (Mesh& m : meshes) m.EnableInstancing(instanceDataBuffer, sizeofSingleComponent, numComponents);
    }

    void Delete() {
        for (auto& mesh : meshes) {
            mesh.Delete();
        }
        //cachedShader.Delete(); // other meshes may be using the same shader...
    }
    inline bool isValid() const { return !meshes.empty(); }

    Mesh* GetMesh(const std::string& name);

    Shader cachedShader;
    std::vector<Mesh> meshes = {};
};


#endif