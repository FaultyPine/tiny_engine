#include "mesh.h"

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<u32>& idxs, const std::vector<Texture>& texs) {
    vertices = verts; indices = idxs; textures = texs;
    initMesh();
}
Mesh::Mesh(const Shader& shader, const std::vector<Vertex>& verts, const std::vector<u32>& idxs, const std::vector<Texture>& texs) {
    vertices = verts; indices = idxs; textures = texs; cachedShader = shader;
    initMesh();
}
void Mesh::UnloadMesh() {
    GLCall(glDeleteVertexArrays(1, &VAO));
    GLCall(glDeleteBuffers(1, &VBO));
    GLCall(glDeleteBuffers(1, &EBO));
}

void ConfigureVertexAttrib(u32 attributeLoc, u32 attributeSize, u32 oglType, bool shouldNormalize, u32 stride, void* offset) {
    GLCall(glVertexAttribPointer(attributeLoc, attributeSize, oglType, shouldNormalize ? GL_TRUE : GL_FALSE, stride, offset));
    GLCall(glEnableVertexAttribArray(attributeLoc));
}

void Mesh::initMesh() {
    // create buffers
    GLCall(glGenVertexArrays(1, &VAO));
    GLCall(glGenBuffers(1, &VBO));
    GLCall(glGenBuffers(1, &EBO));

    // bind buffers
    GLCall(glBindVertexArray(VAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));

    // put data into buffers
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));  
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW));

    // tell ogl where vertex attributes are
    ConfigureVertexAttrib( // vert positions
        0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    ConfigureVertexAttrib( // vert normals
        1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    ConfigureVertexAttrib( // vert tex coords
        2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    ConfigureVertexAttrib( // vert color
        3, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // unbind vert array
    GLCall(glBindVertexArray(0));
}

void Mesh::DrawMesh(Shader& shader) {
    assert(textures.size() <= 32); // ogl max texture samplers
    shader.use();
    // setup textures before drawing
    std::vector<u32> numOfEachTexType(TextureMaterialType::NUM_TYPES-1, 1);

    for (s32 i = 0; i < textures.size(); i++) {
        const Texture& tex = textures[i];
        // before binding, activate the texture we're talking about
        GLCall(glActiveTexture(GL_TEXTURE0 + i));

        // get the string representation of our texture type
        // something like tex_diffuse or tex_normal
        std::string texName = GetTexMatTypeString(tex.type);
        // get (and post-increment) the str representation of the number
        // of these kinds of textures we've set.
        // this means shader uniforms will follow the convention of
        // tex_<texture type><number>   I.E. tex_diffuse1
        std::string texNum = std::to_string(numOfEachTexType[tex.type]++);
        
        const char* uniformName = (texName + texNum).c_str();
        // set the texture uniform to the proper texture unit
        shader.setUniform(uniformName, i);
        GLCall(glBindTexture(GL_TEXTURE_2D, tex.id));
    }

    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    GLCall(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex
}