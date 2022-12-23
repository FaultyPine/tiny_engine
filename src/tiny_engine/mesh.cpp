#include "mesh.h"
#include "tiny_engine/camera.h"
#include "tiny_engine/math.h"

Mesh::Mesh(const std::vector<Vertex>& verts, 
            const std::vector<u32>& idxs, 
            const std::vector<Material>& mats) {
    vertices = verts; indices = idxs; materials = mats;
    if (materials.size() == 0) {
        // if no materials, push a default one
        Material defaultMat = Material();
        materials.push_back(defaultMat);
    }
    initMesh();
}
void Mesh::Delete() {
    GLCall(glDeleteVertexArrays(1, &VAO));
    GLCall(glDeleteBuffers(1, &VBO));
    GLCall(glDeleteBuffers(1, &EBO));
    for (auto& m : materials) {
        m.Delete();
    }
}

void ConfigureVertexAttrib(u32 attributeLoc, u32 attributeSize, u32 oglType, bool shouldNormalize, u32 stride, void* offset) {
    GLCall(glVertexAttribPointer(attributeLoc, attributeSize, oglType, shouldNormalize ? GL_TRUE : GL_FALSE, stride, offset));
    // glEnableVertexAttribArray enables vertex attribute for currently bound vertex array object
    // glEnableVertexArrayAttrib ^ but you provide the vertex array obj explicitly
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
    if (!indices.empty()) {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    }

    // put data into buffers
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));  
    if (!indices.empty()) {
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW));
    }

    // tell ogl where vertex attributes are
    ConfigureVertexAttrib( // vert positions
        0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    ConfigureVertexAttrib( // vert normals
        1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    ConfigureVertexAttrib( // vert tex coords
        2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    ConfigureVertexAttrib( // vert color
        3, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));
    ConfigureVertexAttrib( // material id
        4, 1, GL_INT, false, sizeof(Vertex), (void*)offsetof(Vertex, materialId));

    // unbind vert array
    GLCall(glBindVertexArray(0));
}

void Set3DMatrixUniforms(const Shader& shader, glm::vec3 position, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis) {
    Camera& cam = Camera::GetMainCamera();
    
    // identity matrix to start out with
    glm::mat4 model = Math::Position3DToModelMat(position, scale, rotation, rotationAxis);
    glm::mat4 view = cam.GetViewMatrix();
    glm::mat4 projection = cam.GetProjectionMatrix();

    shader.use();
    // set transform uniforms with the raw data of our matricies
    shader.setUniform("mvp", projection * view * model);
}

void Mesh::Draw(const Shader& shader, glm::vec3 position, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis) const {
    if (!isValid()) {
        std::cout << "[ERR] Tried to draw invalid mesh!\n";
        return;
    }
    shader.use();
    // misc uniforms
    shader.setUniform("nearClip", Camera::GetMainCamera().nearClip);
    shader.setUniform("farClip", Camera::GetMainCamera().farClip);
    // mvp uniform
    Set3DMatrixUniforms(shader, position, scale, rotation, rotationAxis);
    // material uniforms
    for (u32 i = 0; i < materials.size(); i++) {
        materials.at(i).SetShaderUniforms(shader, i);
    }

    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    if (!indices.empty()) {
        GLCall(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    }
    else {
        GLCall(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));
    }
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex

}
void Mesh::Draw(const Shader& shader, const glm::mat4& mvp) const {
    if (!isValid()) {
        std::cout << "[ERR] Tried to draw invalid mesh!\n";
        return;
    }
    shader.use();
    // misc uniforms
    shader.setUniform("nearClip", Camera::GetMainCamera().nearClip);
    shader.setUniform("farClip", Camera::GetMainCamera().farClip);
    // mvp uniform
    shader.setUniform("mvp", mvp);
    // material uniforms
    for (u32 i = 0; i < materials.size(); i++) {
        materials.at(i).SetShaderUniforms(shader, i);
    }

    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    if (!indices.empty()) {
        GLCall(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    }
    else {
        GLCall(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));
    }
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex
}