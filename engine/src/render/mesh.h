#ifndef MESH_H
#define MESH_H

//#include "pch.h"
#include "math/tiny_math.h"
#include "texture.h"
#include "tiny_types.h"
#include <vector>

 
struct Vertex {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 texCoords = glm::vec3(0);
    glm::vec3 color = glm::vec3(1);
    inline std::string str() {
        return "[Pos = " + glm::to_string(position) + "  Normals = " + glm::to_string(normal) + "  TexCoords = " + glm::to_string(texCoords) + "  Colors = " + glm::to_string(color) + " ]";
    }
};
struct Mesh {
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
    TAPI void Draw(const Shader& shader) const;
    TAPI void EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents);
   
private:    
    void initMesh();
};




#endif