//#include "pch.h"
#include "mesh.h"
#include "camera.h"
#include "math/tiny_math.h"
#include "tiny_engine.h"
#include "tiny_ogl.h"
#include "shader.h"
#include "tiny_profiler.h"
#include "render/tiny_renderer.h" // TODO: remove when renderer is integrated

Mesh::Mesh(
    const std::vector<Vertex>& verts, 
    const std::vector<u32>& idxs, 
    const Material& mat,
    const std::string& name
) {
    PROFILE_FUNCTION();
    material = mat;
    vertices = verts; 
    indices = idxs;
    this->name = name;
    initMesh();
    cachedBoundingBox = CalculateMeshBoundingBox();
}
void Mesh::Delete() {
    GLCall(glDeleteVertexArrays(1, &VAO));
    GLCall(glDeleteBuffers(1, &VBO));
    GLCall(glDeleteBuffers(1, &EBO));
    glDeleteBuffers(1, &instanceData.instanceVBO);
    //free(instanceData);
    //material.Delete();
}


void Mesh::EnableInstancing(void* instanceDataBuffer, u32 stride, u32 numElements) 
{
    // TODO: remove this line once new renderer is integrated. Need it for instanced meshes that need to go through prepass shader
    u32 instanceVBO = 0;
    Renderer::EnableInstancing(VAO, instanceDataBuffer, stride, numElements, vertexAttributeLocation, instanceVBO);
    instanceData.instanceData = instanceDataBuffer;
    instanceData.numInstances = numElements;
    instanceData.stride = stride;
}

void ConfigureMeshVertexAttributes(u32& vertexAttributeLocation)
{
    // bind vertex attributes to currently bound VAO
    // also stores reference to currently bound VBO when glVertexAttribPointer is called
    ConfigureVertexAttrib( // vert positions
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position)); // 0
    ConfigureVertexAttrib( // vert normals
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal)); // 1
    ConfigureVertexAttrib( // vert tangent
        vertexAttributeLocation++, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, tangent)); // 2
    ConfigureVertexAttrib( // vert tex coords
        vertexAttributeLocation++, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)); // 3
    ConfigureVertexAttrib( // vert color
        vertexAttributeLocation++, 4, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color)); // 4
    ConfigureVertexAttrib( // object id
        vertexAttributeLocation++, 1, GL_UNSIGNED_INT, false, sizeof(Vertex), (void*)offsetof(Vertex, objectID)); // 5
}

void Mesh::initMesh() 
{
    PROFILE_FUNCTION();
    // create
    GLCall(glGenVertexArrays(1, &VAO));
    GLCall(glGenBuffers(1, &VBO));
    GLCall(glGenBuffers(1, &EBO));
    // bind
    GLCall(glBindVertexArray(VAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    if (!indices.empty()) 
    {
        // making sure to bind this AFTER the vao is bound
        // since the EBO (and VBO) actually ends up stored inside the VAO
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    }
    // upload data into VBO
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));  
    if (!indices.empty()) 
    {
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW));
    }
    // vertex attributes
    ConfigureMeshVertexAttributes(vertexAttributeLocation);
    // unbind
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
}

void Mesh::ReuploadToGPU()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (!indices.empty()) 
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    }
    // NOTE: using glBufferData, not glBufferSubData since we may have added more vertices, so we should do a full realloc
    // would be better to track both cases but this is an edge case anyway
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  
    if (!indices.empty()) 
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW);
    }
}

void Mesh::Draw() const 
{
    PROFILE_FUNCTION();
    TINY_ASSERT(isValid() && "[ERR] Tried to draw invalid mesh!");
    if (!isVisible) return;
    if (instanceData.numInstances > 0)
    {
        OGLDrawInstanced(VAO, indices.size(), vertices.size(), instanceData.numInstances);
    }
    else
    {
        OGLDrawDefault(VAO, indices.size(), vertices.size());
    }
}



BoundingBox Mesh::CalculateMeshBoundingBox() 
{
    PROFILE_FUNCTION();
    // Get min and max vertex to construct bounds (AABB)
    glm::vec3 minVertex = glm::vec3(0);
    glm::vec3 maxVertex = glm::vec3(0);

    if (!vertices.empty()) 
    {
        minVertex = glm::vec3(vertices[0].position);
        maxVertex = glm::vec3(vertices[0].position);

        for (s32 i = 1; i < vertices.size(); i++) 
        {
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