#ifndef TINY_MODEL_H
#define TINY_MODEL_H

#include "pch.h"
#include "mesh.h"
#include "tiny_lights.h"

// TODO: move lights array out of model
// pass it into the draw function optionally
// can then draw models with or without lighting

struct Model {
    Model(){}
    Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir);

    void Draw(glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1,0,0}, const std::vector<Light>& lights = {}) const;
    void Draw(const Shader& shader, glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1,0,0}, const std::vector<Light>& lights = {}) const;
    void Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights = {}) const;
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