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
    u32 VAO, VBO, EBO; // vert array obj, vert buf obj, element buf obj
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    //std::vector<Texture> textures = {};
    std::vector<Material> materials = {};
    
    Mesh(){}
    Mesh( const std::vector<Vertex>& verts, 
        const std::vector<u32>& idxs, 
        const std::vector<Material>& mats = {});
    void Delete();
    inline bool isValid() const {
        return vertices.size() && VAO;
    }

    // draw mesh with specified shader
    void Draw(const Shader& shader, const Transform& tf) const;
    void Draw(const Shader& shader, const glm::mat4& mvp) const;
    void DrawInstanced(const Shader& shader, const std::vector<Transform>& transforms) const;
   
private:    
    void initMesh();
};




#endif