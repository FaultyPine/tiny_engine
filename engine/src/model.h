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

    BoundingBox CalculateBoundingBox();

    TAPI void Draw(const Shader& shader, const Transform& tf) const;
    void Draw(const Transform& tf) const 
    {
        Draw(cachedShader, tf);
    }

    TAPI void DrawMinimal(const Shader& shader) const;
    void DrawMinimal() const 
    {
        DrawMinimal(cachedShader);
    }

    // sends instance data to gpu and "marks" all submeshes as instanced
    void EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents) 
    {
        for (Mesh& m : meshes) 
        {
            m.EnableInstancing(instanceDataBuffer, sizeofSingleComponent, numComponents);
        }
    }

    void Delete() 
    {
        for (auto& mesh : meshes) 
        {
            mesh.Delete();
        }
        //cachedShader.Delete(); // other meshes may be using the same shader...
    }
    inline bool isValid() const { return !meshes.empty(); }

    TAPI Mesh* GetMesh(const std::string& name);

    BoundingBox cachedBoundingBox = {};
    Shader cachedShader = {};
    std::vector<Mesh> meshes = {};
};


#endif