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

    void Draw(glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1,0,0}, const std::vector<Light>& lights = {});
    void Draw(const Shader& shader, glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1,0,0}, const std::vector<Light>& lights = {});
    inline bool isValid() { return !meshes.empty(); }
    inline void SetIsOverrideMatrix(bool shouldOverride) {
        for (Mesh& m : meshes) {
            m.isOverrideModelMatrix = shouldOverride;
        }
    }

    std::vector<Mesh> meshes = {};
};


#endif