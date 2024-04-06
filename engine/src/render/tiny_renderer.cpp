#include "tiny_renderer.h"

#include "mem/tiny_arena.h"
#include "containers/fixed_growable_array.h"
#include "tiny_engine.h"
#include "render/shader.h"
#include "render/tiny_ogl.h"
#include "camera.h"
#include "tiny_profiler.h"
#include "render/mesh.h"
#include "render/shader.h"
#include "tiny_types.h"
#include "render/model.h"
#include "render/tiny_lights.h"
#include "scene/entity.h"
#include "res/shaders/shader_defines.glsl"

// TODO:
// consolidate shape shaders(?)

// renderable shapes
struct RPoint
{
    SimpleVertex vert;
};
struct RLine
{
    SimpleVertex start, end;
};
struct RTriangle
{
    SimpleVertex a, b, c;
};

typedef Vertex RMeshVertex;
typedef u32 RMeshIndex;
struct RMesh
{
    BufferView<RMeshVertex> vertices = {};
    BufferView<RMeshIndex> indices = {};
    u32 numInstances = 0;
};

struct MeshBatch
{
    // batches must share the same material/shader
    Material material = {};
    Shader shader = {};
    FixedGrowableArray<RMesh, 100> meshes = {};
    u32 batchVAO, batchVBO, batchEBO, vertexAttributeLocation = 0;
    bool dirty = true;
};
void ClearMeshBatch(MeshBatch& batch)
{
    // making sure we do *not* clear the VAO/VBO data
    batch.meshes.clear();
    batch.material = {};
    batch.shader = {};
}

// hash required data for a batch. Inputs that hash the same can be batched
static u64 MeshBatchHash(const Material& material, const Shader& shader)
{
    u64 result = 0;
    // bottom 32 bits are material id, top 32 bits are shader id
    result |= MaterialHasher()(material);
    result |= ShaderHasher()(shader) << 32;
    result = HashBytesL((u8*)&result, sizeof(result));
    return result;
}

struct RendererData
{
    Arena arena = {};
    FixedGrowableArray<RPoint, 300> points = {};
    u32 pointsVAO, pointsVBO = 0;
    FixedGrowableArray<RLine, 300> lines = {};
    u32 linesVAO, linesVBO = 0;
    FixedGrowableArray<RTriangle, 300> triangles = {};
    u32 trianglesVAO, trianglesVBO = 0;
    typedef std::unordered_map<u64, MeshBatch> MeshMap;
    MeshMap models = {};
};

namespace Renderer
{

static Shader defaultShapeShader;

void InitializeRenderer(Arena* arena)
{
    RendererData* rendererMem = arena_alloc_and_init<RendererData>(arena);
    GetEngineCtx().renderer = rendererMem;
    u32 renderingDataSize = MEGABYTES_BYTES(100);
    rendererMem->arena = arena_init(arena_alloc(arena, renderingDataSize), renderingDataSize, "Rendering Data");

    if (!defaultShapeShader.isValid()) 
    {
        defaultShapeShader = Shader::CreateShaderFromStr(
R"(
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec4 vertColor;
out vec4 color;
uniform mat4 mvp;
void main(){
    color = vertColor;
	gl_Position = mvp * vec4(vertPos, 1.0);
}
)",
R"(
out vec4 fragColor;
in vec4 color;
void main(){
	fragColor = color;
}
)"
        );
    }
}

RendererData& GetRenderer()
{
    return *GetEngineCtx().renderer;
}

static void DrawPoints(RendererData& renderer, f32 pointSize)
{
    u32 numPoints = renderer.points.size;
    RPoint* points = renderer.points.get_elements();
    if (numPoints == 0) return;
    u32& VAO = renderer.pointsVAO;
    u32& VBO = renderer.pointsVBO;
    if (VAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RPoint) * numPoints, points, GL_DYNAMIC_DRAW));
        // vec3 vertPos vec4 vertColor
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, position)));
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, color)));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RPoint) * numPoints, points));
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1);
    glm::mat4 mvp = proj * view * model;
    defaultShapeShader.setUniform("mvp", mvp);
    defaultShapeShader.use();
    GLCall(glBindVertexArray(VAO));
    glPointSize(pointSize);
    GLCall(glDrawArrays(GL_POINTS, 0, numPoints));
}

void DrawLines(RendererData& renderer)
{
    u32 numLines = renderer.lines.size;
    RLine* lines = renderer.lines.get_elements();
    if (numLines == 0) return;
    u32& VAO = renderer.linesVAO;
    u32& VBO = renderer.linesVBO;
    if (VAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RLine) * numLines, lines, GL_DYNAMIC_DRAW));
        // this shader has vert attributes: vec3 vertPos vec4 vertColor
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, position)));
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, color)));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RLine) * numLines, lines));
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    defaultShapeShader.setUniform("mvp", mvp);
    defaultShapeShader.use();
    GLCall(glBindVertexArray(VAO));
    GLCall(glDrawArrays(GL_LINES, 0, numLines*2)); // *2 b/c count represents "number of indices to be rendered"
}

void DrawTriangles(RendererData& renderer)
{
    u32 numTriangles = renderer.triangles.size;
    RTriangle* triangles = renderer.triangles.get_elements();
    if (numTriangles == 0) return;
    u32& quadVAO = renderer.trianglesVAO;
    u32& VBO = renderer.trianglesVBO;
    if (quadVAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &quadVAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RTriangle) * numTriangles, triangles, GL_DYNAMIC_DRAW));
        // this shader has vert attributes: vec3 vertPos vec4 vertColor
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, position)));
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, color)));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RTriangle) * numTriangles, triangles));
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    defaultShapeShader.setUniform("mvp", mvp);
    defaultShapeShader.use();
    GLCall(glBindVertexArray(quadVAO));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, numTriangles*3));
}

struct DrawElementsIndirectCommand 
{
    u32  count;
    u32  instanceCount;
    u32  firstIndex;
    s32  baseVertex;
    u32  baseInstance;
};

void DrawModels(RendererData& renderer, Arena* arena)
{
    // TODO: use arena instead of std::vector
    Camera& cam = Camera::GetMainCamera();
    // each batch shares the *exact* same material and shader

    for (auto& [batchHash, batch] : renderer.models)
    {
        const char* materialName = GetMaterialInternal(batch.material).name;
        //LOG_INFO("Batch %s  %llu", materialName, batchHash);
        // collect data about this batch
        std::vector<s32> vertexBaseOffsets = {}; // offsets into the gpu buffer for multidraw
        std::vector<size_t> idxOffsetsBytes = {}; // offsets into the gpu buffer for multidraw
        std::vector<s32> elementCounts = {}; // number of elements to be rendered

        size_t currentVertOffset = 0;
        size_t verticesMemSize = 0; // how much memory do we need for this batch? (used for init)
        size_t indicesMemSize = 0;
        for (u32 i = 0; i < batch.meshes.size; i++)
        {
            const RMesh& mesh = batch.meshes.at(i);
            u32 numIndices = mesh.indices.size / mesh.indices.stride();
            u32 numVertices = mesh.vertices.size / mesh.vertices.stride();

            elementCounts.push_back(numIndices);
            vertexBaseOffsets.push_back(currentVertOffset);
            idxOffsetsBytes.push_back(indicesMemSize);

            verticesMemSize += mesh.vertices.size;
            indicesMemSize += mesh.indices.size;
            currentVertOffset += numVertices;
        }

        u32& VAO = batch.batchVAO;
        u32& VBO = batch.batchVBO;
        u32& EBO = batch.batchEBO;
        u32& vertexAttributeLocation = batch.vertexAttributeLocation;
        if (batch.batchVAO == 0)
        { // lazy init
            LOG_INFO("allocating %f vertices mb  and %f indices kb", (f64)verticesMemSize / 1000.0 / 1000.0, (f64)indicesMemSize / 1000.0);
            // initialize gpu buffers
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            if (indicesMemSize > 0) 
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            }
            // allocate (not copying over yet)
            glBufferData(GL_ARRAY_BUFFER, verticesMemSize, NULL, GL_STATIC_DRAW);
            if (indicesMemSize > 0)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesMemSize, NULL, GL_STATIC_DRAW);
            }
            // bind vertex attributes to VAO
            ConfigureMeshVertexAttributes(vertexAttributeLocation);
        }
        
        if (batch.dirty)
        {
            // dirty means we need to upload to gpu
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            if (EBO != 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            }
            // here's the meat and potatoes of batching
            // we've allocated a sufficiently sized gpu buffer for all our verts & idxs
            // now we need to copy them to that buffer. They are spread around in host mem, need to collect them each
            // and upload with proper offsets. In the future this should only re-upload "dirty" meshes
            for (u32 i = 0; i < batch.meshes.size; i++)
            {
                const RMesh& mesh = batch.meshes.at(i);
                size_t vertOffset = vertexBaseOffsets[i]*sizeof(RMeshVertex);
                glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)vertOffset, mesh.vertices.size, mesh.vertices.data);
                if (EBO != 0)
                {
                    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)idxOffsetsBytes[i], mesh.indices.size, mesh.indices.data);
                }
            }
            batch.dirty = false;
        }
        const Shader& shader = batch.shader;
        const Material& material = batch.material;
        SetLightingUniforms(shader);
        material.SetShaderUniforms(shader);
        shader.use();
        u32 drawCount = batch.meshes.size;
        u32 numInstances = 0; // TODO: support instanced
        if (numInstances > 0)
        {
            // instanced render
            //OGLDrawInstanced(VAO, indices.size(), vertices.size(), numInstances);

            //glDrawElementsInstancedBaseVertexBaseInstance(
            //    GL_TRIANGLES, );
        }
        else
        {
            glBindVertexArray(VAO);
            TINY_ASSERT(elementCounts.size() == drawCount && vertexBaseOffsets.size() == drawCount);
            if (EBO != 0)
            {
                // TODO: glMultiDrawElementsIndirect
                //LOG_INFO("Drawcalls in batch: %i", drawCount);
                glMultiDrawElementsBaseVertex(
                    GL_TRIANGLES, (s32*)elementCounts.data(), GL_UNSIGNED_INT, (const void *const*)idxOffsetsBytes.data(), drawCount, (s32*)vertexBaseOffsets.data());
            }
            else
            {
                //GLCall(glMultiDrawArrays(GL_TRIANGLES, (s32*)vertexBaseOffsets.data(), (s32*)elementCounts.data(), drawCount));
            }
        }
        ClearMeshBatch(batch);
    }
}

void RendererDraw()
{
    PROFILE_FUNCTION();
    RendererData& renderer = GetRenderer();
    DrawPoints(renderer, 10.0f);
    renderer.points.clear();
    DrawLines(renderer);
    renderer.lines.clear();
    DrawTriangles(renderer);
    renderer.triangles.clear();
    DrawModels(renderer, &renderer.arena);
}

void PushPoint(const glm::vec3& point, const glm::vec4& color)
{
    RendererData& renderer = GetRenderer();
    RPoint rpoint;
    rpoint.vert.position = point;
    rpoint.vert.color = color;
    renderer.points.push_back(rpoint);
}

void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
{
    RendererData& renderer = GetRenderer();
    RLine rline;
    rline.start = {start, color};
    rline.end = {end, color};
    renderer.lines.push_back(rline);
}

void PushTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color)
{
    RendererData& renderer = GetRenderer();
    RTriangle tri;
    tri.a = {a, color};
    tri.b = {b, color};
    tri.c = {c, color};
    renderer.triangles.push_back(tri);
}

void PushEntity(const EntityRef& entity)
{
    EntityData& entityData = Entity::GetEntity(entity);
    const Model& model = entityData.model;
    const Shader& entityShader = model.cachedShader;
    RendererData& renderer = GetRenderer();
    for (u32 i = 0; i < model.meshes.size(); i++)
    {
        const Mesh& mesh = model.meshes[i];
        if (!mesh.isVisible) continue;
        RMesh rmesh;
        rmesh.vertices = {const_cast<RMeshVertex*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(RMeshVertex)};
        rmesh.indices = {const_cast<RMeshIndex*>(mesh.indices.data()), mesh.indices.size() * sizeof(RMeshIndex)};
        rmesh.numInstances = mesh.numInstances;
        // batches are bucketed by hashing their shader and material together
        u64 rmeshHash = MeshBatchHash(mesh.material, entityShader);
        MeshBatch& batch = renderer.models[rmeshHash];
        TINY_ASSERT(!batch.material.isValid() || batch.material == mesh.material); // either we are adding for the first time or they must be the same
        TINY_ASSERT(!batch.shader.isValid() || batch.shader == entityShader); // either we are adding for the first time or they must be the same
        batch.material = mesh.material;
        batch.shader = entityShader;
        //LOG_INFO("Pushed. Mat = %u %s shader = %u   batch hash = %llu", batch.material.id, GetMaterialInternal(batch.material).name, batch.shader.ID, rmeshHash);
        batch.meshes.push_back(rmesh);
    }
}

void PushDebugRenderMarker(const char* name)
{
    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
}
void PopDebugRenderMarker()
{
    glPopDebugGroupKHR();
}

} // namespace Renderer