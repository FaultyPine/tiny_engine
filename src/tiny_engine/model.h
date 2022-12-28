#ifndef TINY_MODEL_H
#define TINY_MODEL_H

#include "pch.h"
#include "mesh.h"
#include "tiny_lights.h"

struct Model {
    Model(){}
    Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir);
    Model(const Shader& shader, const std::vector<Mesh>& meshes);

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
    
    // instanced drawing
    void DrawInstanced(const Shader& shader, const std::vector<Transform>& transforms, const std::vector<Light>& lights = {}) const;
    void DrawInstanced(const std::vector<Transform>& transforms, const std::vector<Light>& lights = {}) const {
        DrawInstanced(cachedShader, transforms, lights);
    }

    void Delete() {
        for (auto& mesh : meshes) {
            mesh.Delete();
        }
        //cachedShader.Delete(); // other meshes may be using the same shader...
    }
    inline bool isValid() { return !meshes.empty(); }

    Shader cachedShader;
    std::vector<Mesh> meshes = {};
};


#endif