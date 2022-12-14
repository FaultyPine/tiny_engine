#ifndef MESH_H
#define MESH_H

#include "pch.h"
#include "shader.h"
#include "texture.h"

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
    // TODO: might be cleaner to have the shader cached in the Model class
    // and have a Mesh always take a shader to be drawn with
    Shader cachedShader;
    u32 VAO, VBO, EBO; // vert array obj, vert buf obj, element buf obj
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    //std::vector<Texture> textures = {};
    std::vector<Material> materials = {};
    // when true, the shader MVP is not set
    bool isOverrideModelMatrix = false;
    
    Mesh(){}
    Mesh(const Shader& shader, 
        const std::vector<Vertex>& verts, 
        const std::vector<u32>& idxs, 
        const std::vector<Material>& mats = {});
    void UnloadMesh();
    inline void SetMeshShader(const Shader& shader) { cachedShader = shader; }
    inline bool isValid() const {
        return vertices.size() && indices.size() && VAO && VBO && EBO;
    }
    inline Shader& GetShader() { return cachedShader; }

    // draw mesh with specified shader
    inline void Draw(const Shader& shader, glm::vec3 position, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1.0, 0.0, 0.0}) const {
        DrawMesh(shader, position, scale, rotation, rotationAxis);
    }
    // draw mesh with cached shader
    inline void Draw(glm::vec3 position, glm::vec3 scale = glm::vec3(1.0), f32 rotation = 0.0, glm::vec3 rotationAxis = {1.0, 0.0, 0.0}) const {
        DrawMesh(cachedShader, position, scale, rotation, rotationAxis);
    }
   

private:
    
    void DrawMesh(const Shader& shader, glm::vec3 position, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis) const;
    void initMesh();
};




#endif