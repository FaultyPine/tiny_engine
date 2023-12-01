//#include "pch.h"
#include "mesh.h"
#include "camera.h"
#include "math/tiny_math.h"
#include "tiny_engine.h"
#include "tiny_ogl.h"
#include "shader.h"
#include "tiny_profiler.h"

Mesh::Mesh(
    const std::vector<Vertex>& verts, 
    const std::vector<u32>& idxs, 
    const Material& mat,
    const std::string& name
) {
    PROFILE_FUNCTION();
    vertices = verts; indices = idxs; material = mat; this->name = name;
    initMesh();
    cachedBoundingBox = CalculateMeshBoundingBox();
}
void Mesh::Delete() {
    GLCall(glDeleteVertexArrays(1, &VAO));
    GLCall(glDeleteBuffers(1, &VBO));
    GLCall(glDeleteBuffers(1, &EBO));
    //material.Delete();
}

// Note: numBytesPerComponent must be between [1, 4]
void ConfigureVertexAttrib(u32 attributeLoc, u32 numBytesPerComponent, u32 oglType, bool shouldNormalize, u32 stride, void* offset) {
    // this retrieves the value of GL_ARRAY_BUFFER (VBO) and associates it with the current VAO
    GLCall(glVertexAttribPointer(attributeLoc, numBytesPerComponent, oglType, shouldNormalize ? GL_TRUE : GL_FALSE, stride, offset));
    // glEnableVertexAttribArray enables vertex attribute for currently bound vertex array object
    // glEnableVertexArrayAttrib ^ but you provide the vertex array obj explicitly
    GLCall(glEnableVertexAttribArray(attributeLoc));
}

void Mesh::EnableInstancing(void* instanceDataBuffer, u32 sizeofSingleComponent, u32 numComponents) {
    PROFILE_FUNCTION();
    if (instanceVBO != 0) return;
    this->numInstances = numComponents;
    GLCall(glBindVertexArray(VAO));
    GLCall(glGenBuffers(1, &instanceVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, instanceVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeofSingleComponent*numComponents, instanceDataBuffer, GL_STATIC_DRAW));
    // set up vertex attribute(s) for instance-specific data
    // if we want float/vec2/vec3/vec4 its not a big deal,
    // just send that into a vertex attribute as normal
    // but if its something like a mat4, it'll take up multiple
    // vertex attribute slots since we can only pass a max of 4 floats in one attribute
    // TODO: verify this works for all data types (float, vec2/3/4, mat2/3/4)
    u32 numFloatsInComponent = sizeofSingleComponent / 4;
    u64 numVec4sInComponent = std::max(1u, numFloatsInComponent / 4);
    for (u64 i = 0; i < numVec4sInComponent; i++) {
        ConfigureVertexAttrib( // instance data
            vertexAttributeLocation+i, numFloatsInComponent / 4, GL_FLOAT, false, sizeofSingleComponent, (void*)(i*numFloatsInComponent));
        // update vertex attribute on every new instance of the mesh, not on every vertex (1 is a magic opengl number corresponding to "update-per-instance", rather than update-per-vertex)
        GLCall(glVertexAttribDivisor(vertexAttributeLocation+i, 1));  
    }
    vertexAttributeLocation += numVec4sInComponent;
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
}

void Mesh::initMesh() {
    PROFILE_FUNCTION();
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

    // put data into VBO
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));  
    if (!indices.empty()) {
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW));
    }

    // bind vertex attributes to VAO
    // also stores reference to VBO when glVertexAttribPointer is called
    ConfigureVertexAttrib( // vert positions
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position)); // 0
    ConfigureVertexAttrib( // vert normals
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal)); // 1
    ConfigureVertexAttrib( // vert tangent
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, tangent)); // 2
    ConfigureVertexAttrib( // vert tex coords
        vertexAttributeLocation++, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)); // 3
    ConfigureVertexAttrib( // vert color
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color)); // 4

    // unbind
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
}

void OGLDrawDefault(u32 VAO, u32 indicesSize, u32 verticesSize) {
    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    if (indicesSize) { // if indices is not empty, draw indexed
        GLCall(glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0));
    }
    else { // indices is empty, draw arrays
        GLCall(glDrawArrays(GL_TRIANGLES, 0, verticesSize));
    }
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex
}
void OGLDrawInstanced(u32 VAO, u32 indicesSize, u32 verticesSize, u32 numInstances) {
    // draw mesh = bind vert array -> draw -> unbind
    GLCall(glBindVertexArray(VAO));
    if (indicesSize) { // if indices is not empty, draw indexed
        GLCall(glDrawElementsInstanced(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0, numInstances));
    }
    else { // indices is empty, draw arrays
        GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, verticesSize, numInstances));
    }
    
    // clean up
    GLCall(glBindVertexArray(0)); // unbind vert array
    GLCall(glActiveTexture(GL_TEXTURE0)); // reset active tex
}


void Mesh::Draw(const Shader& shader) const {
    PROFILE_FUNCTION();
    TINY_ASSERT(isValid() && "[ERR] Tried to draw invalid mesh!");
    if (!isVisible) return;
    if (numInstances > 0)
    {
        // instanced render
        TINY_ASSERT(instanceVBO != 0 && "Tried to instance mesh that has not had EnableInstance called");
        OGLDrawInstanced(VAO, indices.size(), vertices.size(), numInstances);
    }
    else
    {
        OGLDrawDefault(VAO, indices.size(), vertices.size());
    }
}



BoundingBox Mesh::CalculateMeshBoundingBox() {
    PROFILE_FUNCTION();
    // Get min and max vertex to construct bounds (AABB)
    glm::vec3 minVertex = glm::vec3(0);
    glm::vec3 maxVertex = glm::vec3(0);

    if (!vertices.empty()) {
        minVertex = glm::vec3(vertices[0].position);
        maxVertex = glm::vec3(vertices[0].position);

        for (s32 i = 1; i < vertices.size(); i++) {
            minVertex = glm::min(minVertex, vertices[i].position);
            maxVertex = glm::max(maxVertex, vertices[i].position);
        }
    }

    // Create the bounding box
    BoundingBox box = {};
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}