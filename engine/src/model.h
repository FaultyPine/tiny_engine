#ifndef TINY_MODEL_H
#define TINY_MODEL_H

//#include "pch.h"
#include "render/mesh.h"
#include "render/tiny_lights.h"
#include "render/shader.h"

struct Model {
    Model() = default;
    TAPI Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir);
    TAPI Model(const Shader& shader, const std::vector<Mesh>& meshes);

    BoundingBox GetBoundingBox();

    // draw with transform
    TAPI void Draw(const Shader& shader, const Transform& tf, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const;
    void Draw(const Transform& tf, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const {
        Draw(cachedShader, tf, lights, sun);
    }
    // draw with mvp matrix
    TAPI void Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const;
    void Draw(const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const {
        Draw(cachedShader, mvp, modelMat, lights, sun);
    }

    TAPI void DrawMinimal(const Shader& shader) const;
    
    // instanced drawing
    TAPI void DrawInstanced(const Shader& shader, u32 numInstances, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const;
    void DrawInstanced(u32 numInstances, const std::vector<LightPoint>& lights = {}, LightDirectional sun = {}) const {
        DrawInstanced(cachedShader, numInstances, lights, sun);
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

    TAPI Mesh* GetMesh(const std::string& name);

    Shader cachedShader;
    std::vector<Mesh> meshes = {};
};


#endif