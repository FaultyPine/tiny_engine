#ifndef MESH_H
#define MESH_H

#include "pch.h"
#include "shader.h"
#include "texture.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
    inline std::string str() {
        std::string pos = "("  + std::to_string(position.x)  + ", " + std::to_string(position.y)  + ", " + std::to_string(position.z) + ")"; 
        std::string norm = "(" + std::to_string(normal.x)    + ", " + std::to_string(normal.y)    + ", " + std::to_string(normal.z) + ")";
        std::string tex = "("  + std::to_string(texCoords.x) + ", " + std::to_string(texCoords.y) + ")";
        std::string col = "("  + std::to_string(color.x)     + ", " + std::to_string(color.y)     + ", " + std::to_string(color.z) + ")";
        return "[Pos = " + pos + "  Normals = " + norm + "  TexCoords = " + tex + "  Colors = " + col + " ]";
    }
};
struct Mesh {
    Shader cachedShader;
    u32 VAO, VBO, EBO; // vert array obj, vert buf obj, element buf obj
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;
    
    Mesh(){}
    Mesh(const std::vector<Vertex>& verts, const std::vector<u32>& idxs, const std::vector<Texture>& texs);
    Mesh(const Shader& shader, const std::vector<Vertex>& verts, const std::vector<u32>& idxs, const std::vector<Texture>& texs);
    void UnloadMesh();
    inline void SetCachedShader(const Shader& shader) { cachedShader = shader; }

    // draw mesh with specified shader
    inline void Draw(Shader& shader) {
        DrawMesh(shader);
    }
    // draw mesh with cached shader
    inline void Draw() {
        DrawMesh(cachedShader);
    }
   
    void initMesh();

private:
    
    void DrawMesh(Shader& shader);
};




#endif