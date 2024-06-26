#ifndef TINY_MODEL_H
#define TINY_MODEL_H

#include "render/mesh.h"
#include "render/tiny_lights.h"
#include "render/shader.h"

struct Model {
    Model() = default;
    TAPI Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir, u32 objectID);
    TAPI Model(const Shader& shader, const std::vector<Mesh>& meshes, u32 objectID);

    BoundingBox CalculateBoundingBox();

    TAPI void Draw(const Shader& shader, const Transform& tf) const;
    void Draw(const Transform& tf) const 
    {
        Draw(cachedShader, tf);
    }

    TAPI void DrawMinimal() const;

    // sends instance data to gpu and "marks" all submeshes as instanced
    void EnableInstancing(void* instanceDataBuffer, u32 stride, u32 numElements) 
    {
        for (Mesh& m : meshes) 
        {
            m.EnableInstancing(instanceDataBuffer, stride, numElements);
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

    template <typename... Args>
    void setUniform(const s8* uniformName, Args... args) const { cachedShader.setUniform(uniformName, args...); }
    void TryAddSampler(const Texture& texture, const char* uniformName) const { cachedShader.TryAddSampler(texture, uniformName); }
    void TryAddSampler(const Cubemap& texture, const char* uniformName) const { cachedShader.TryAddSampler(texture, uniformName); }

    BoundingBox cachedBoundingBox = {};
    Shader cachedShader = {};
    std::vector<Mesh> meshes = {};
};


#endif