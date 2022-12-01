#include "mesh.h"
#include "tiny_engine/camera.h"

Mesh::Mesh(const Shader& shader, 
            const std::vector<Vertex>& verts, 
            const std::vector<u32>& idxs, 
            const std::vector<Material>& mats) {
    vertices = verts; indices = idxs; cachedShader = shader; materials = mats;
    if (materials.size() == 0) {
        // if no materials, set defaults
        Material defaultMat = Material();
        materials.push_back(defaultMat);
    }
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
    ConfigureVertexAttrib( // material id
        4, 1, GL_INT, false, sizeof(Vertex), (void*)offsetof(Vertex, materialId));

    // unbind vert array
    GLCall(glBindVertexArray(0));
}

void Set3DMatrixUniforms(const Shader& shader, glm::vec3 position, f32 scale, f32 rotation, glm::vec3 rotationAxis) {
    Camera& cam = Camera::GetMainCamera();
    MouseInput& mouseInput = MouseInput::GetMouse();
    
    // identity matrix to start out with
    glm::mat4 model = Position3DToModelMat(position, glm::vec3(scale), rotation, rotationAxis);
    glm::mat4 view = cam.GetViewMatrix();
    glm::mat4 projection = cam.GetProjectionMatrix();

    shader.use();
    // set transform uniforms with the raw data of our matricies
    shader.setUniform("mvp", projection * view * model);
}

void Mesh::DrawMesh(const Shader& shader, glm::vec3 position, f32 scale, f32 rotation, glm::vec3 rotationAxis) const {
    //ASSERT(textures.size() <= 32); // ogl max texture samplers
    if (!isValid()) {
        std::cout << "[ERR] Tried to draw invalid mesh!\n";
        return;
    }
    shader.use();
    Set3DMatrixUniforms(shader, position, scale, rotation, rotationAxis);
    for (u32 i = 0; i < materials.size(); i++) {
        materials.at(i).SetShaderUniforms(shader, i);
    }
    #if 0 // disabled/unused texture code... textures generally reside in the materials now
    // setup textures before drawing
    std::vector<u32> numOfEachTexType(TextureMaterialType::NUM_TYPES-1, 1);

    for (s32 i = 0; i < textures.size(); i++) {
        const Texture& tex = textures[i];
        // before binding, activate the texture we're talking about
        tex.activate(i);

        // get the string representation of our texture type
        // something like tex_diffuse or tex_normal
        std::string texName = GetTexMatTypeString(tex.type);
        // get (and post-increment) the str representation of the number
        // of these kinds of textures we've set.
        // this means shader uniforms will follow the convention of
        // tex_<texture type><number>   I.E. tex_diffuse1
        std::string texNum = std::to_string(numOfEachTexType[tex.type]++);

        // set the texture uniform to the proper texture unit
        shader.setUniform((texName + texNum).c_str(), i);
        tex.bind();
    }
    #endif

    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    GLCall(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex

}