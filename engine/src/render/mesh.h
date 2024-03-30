#ifndef MESH_H
#define MESH_H

//#include "pch.h"
#include "math/tiny_math.h"
#include "texture.h"
#include "tiny_types.h"
#include "tiny_material.h"
#include "tiny_renderer.h"
#include <vector>


struct Mesh 
{
    u32 VAO, VBO, EBO = 0; // vert array obj, vert buf obj, element buf obj
    u32 instanceVBO = 0; // extra vert data for instanced rendering
    u32 numInstances = 0; // nonzero when this mesh has instancing enabled
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
    TAPI void EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents);
    void SetObjectID(u32 objectID)
    {
        for (auto& vertex : vertices)
        {
            vertex.objectID = objectID;
        }
    }
private:    
    void initMesh();
};

void ConfigureMeshVertexAttributes(u32& vertexAttributeLocation);

#endif