#ifndef TINY_MODEL_H
#define TINY_MODEL_H

#include "pch.h"
#include "mesh.h"
#include "tiny_lights.h"

struct Model {
    Model(){}
    Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir);

    void AddLight(const Light& lights);
    void Draw(glm::vec3 pos, f32 scale, f32 rotation, glm::vec3 rotationAxis);
    inline bool isValid() { return !meshes.empty(); }

    std::vector<Mesh> meshes = {};
    std::vector<Light> lights = {};
    bool isLit = false;
};


#endif