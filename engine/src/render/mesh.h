#ifndef MESH_H
#define MESH_H

//#include "pch.h"
#include "math/tiny_math.h"
#include "texture.h"
#include "tiny_types.h"
#include "tiny_material.h"
#include <vector>


struct Vertex 
{
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec3 tangent = glm::vec3(0);
    glm::vec2 texCoords = glm::vec3(0);
    glm::vec4 color = glm::vec4(1);
    u32 objectID = U32_INVALID_ID;
    inline std::string str() 
    {
        return "[Pos = " + glm::to_string(position) + 
                " Normals = " + glm::to_string(normal) + 
                " Tangent = " + glm::to_string(tangent) + 
                " TexCoords = " + glm::to_string(texCoords) + 
                " Colors = " + glm::to_string(color) + 
                " obj id = " + std::to_string(objectID) + 
                " ]";
    }
};
struct SimpleVertex
{
    glm::vec3 position;
    glm::vec4 color;
};

struct GPUInstanceData
{
    void* instanceData = nullptr; // owning - this mesh's (optional) instance data
    u32 instanceVBO = 0; // extra vert data for instanced rendering
    u32 numInstances = 0; // nonzero when this mesh has instancing enabled
    u32 stride = 0;
    bool operator==(const GPUInstanceData& o) { return memcmp(this, &o, sizeof(GPUInstanceData)) == 0; }
};

struct Mesh 
{
    u32 VAO, VBO, EBO = 0; // vert array obj, vert buf obj, element buf obj
    GPUInstanceData instanceData = {};
    u32 vertexAttributeLocation = 0;
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    Material material = {};
    BoundingBox cachedBoundingBox = {};
    std::string name = "";
    bool isVisible = true;
    
    Mesh() = default;
    TAPI Mesh( const std::vector<Vertex>& verts, 
        const std::vector<u32>& idxs = {}, 
        const Material& mat = {},
        const std::string& name = "");
    TAPI void Delete();
    inline bool isValid() const {
        return vertices.size() && VAO;
    }

    BoundingBox CalculateMeshBoundingBox();

    // draw mesh with specified shader
    // NOTE: this function does *not* call use() on the shader
    TAPI void Draw() const;
    /// owns instanceDataBuffer
    TAPI void EnableInstancing(void* instanceDataBuffer, u32 stride, u32 numElements);
    void SetObjectID(u32 objectID)
    {
        for (auto& vertex : vertices)
        {
            vertex.objectID = objectID;
        }
    }
    TAPI void ReuploadToGPU();
private:    
    void initMesh();
};

void ConfigureMeshVertexAttributes(u32& vertexAttributeLocation);

#endif