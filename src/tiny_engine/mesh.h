#ifndef MESH_H
#define MESH_H

#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "tiny_types.h"

struct Vertex {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 texCoords = glm::vec3(0);
    glm::vec3 color = glm::vec3(1);
    u32 materialId = 0;
    inline std::string str() {
        std::string pos = "("  + VecToStr(position) + ")"; 
        std::string norm = "(" + VecToStr(normal) + ", " + std::to_string(normal.z) + ")";
        std::string tex = "("  + VecToStr(texCoords) + ")";
        std::string col = "("  + VecToStr(color) + ")";
        return "[Pos = " + pos + "  Normals = " + norm + "  TexCoords = " + tex + "  Colors = " + col + " ]";
    }
};
struct Mesh {
    u32 VAO, VBO, EBO = 0; // vert array obj, vert buf obj, element buf obj
    u32 instanceVBO = 0; // extra vert data for instanced rendering
    u32 vertexAttributeLocation = 0;
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    //std::vector<Texture> textures = {};
    std::vector<Material> materials = {};
    std::string name = "";
    bool isVisible = true;
    
    Mesh(){}
    Mesh( const std::vector<Vertex>& verts, 
        const std::vector<u32>& idxs, 
        const std::vector<Material>& mats = {},
        const std::string& name = "");
    void Delete();
    inline bool isValid() const {
        return vertices.size() && VAO;
    }

    BoundingBox GetMeshBoundingBox();

    // draw mesh with specified shader
    void Draw(const Shader& shader, const Transform& tf) const;
    void Draw(const Shader& shader, const glm::mat4& mvp) const;
    void MinimalDraw(const Shader& shader) const;
    void EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents);
    void DrawInstanced(const Shader& shader, u32 numInstances) const;
   
private:    
    void initMesh();
};




#endif