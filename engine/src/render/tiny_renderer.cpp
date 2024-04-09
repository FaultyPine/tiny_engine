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
    u32 batchVAO, batchVBO, batchEBO = 0;
    u64 verticesMemSize, indicesMemSize = 0;
    GPUInstanceData instanceData = {};
    bool isInstanced = false; // instanced meshes in a batch have the same "instance data". (model matrices)
    bool dirty = true;
    // when pushing a mesh into a batch, this is set. It is set back to false when the batch is rendered.
    // additionally, the batch is ONLY rendered if this is set. 
    // With this, to draw a mesh every frame you must "push" that mesh to the renderer every frame (through a model or entity or whatever)
    bool active = true;
};

void ClearMeshBatch(MeshBatch& batch)
{
    // making sure we do *not* clear the VAO/VBO data
    batch.meshes.clear();
    batch.material = {};
    batch.shader = {};
    batch.active = false;
}

// hash required data for a batch. Inputs that hash the same can be batched
static u64 MeshBatchHash(const Mesh& mesh, const Shader& shader)
{
    u64 result = 0;
    // bottom 32 bits are material id, top 32 bits are shader id
    result |= MaterialHasher()(mesh.material);
    result |= ShaderHasher()(shader) << 32;
    if (mesh.instanceData.numInstances > 0)
    {
        // cannot batch instanced draws with non-instanced draws. Additionally instance data should be the same for two instanced meshes to be batched
        result ^= HashBytes((u8*)&mesh.instanceData, sizeof(mesh.instanceData)); 
    }
    result = HashBytesL((u8*)&result, sizeof(result));
    return result;
}

struct DrawElementsIndirectCommand 
{
    u32  count;
    u32  instanceCount;
    u32  firstIndex;
    s32  baseVertex;
    u32  baseInstance;
};

constexpr u32 MAX_NUM_MESHES_PER_BATCH = 500; // arbitrary
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
    u32 indirectGPUBuffer = 0;
};

namespace Renderer
{

static Shader defaultShapeShader;

void InitializeRenderer(Arena* arena)
{
    RendererData* rendererMem = arena_alloc_and_init<RendererData>(arena);
    GetEngineCtx().renderer = rendererMem;
    u32 renderingDataSize = Math::PercentOf(get_free_space(arena), 10);
    rendererMem->arena = arena_init(arena_alloc(arena, renderingDataSize), renderingDataSize, "Rendering Data");
    glGenBuffers(1, &rendererMem->indirectGPUBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, rendererMem->indirectGPUBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, MAX_NUM_MESHES_PER_BATCH * sizeof(DrawElementsIndirectCommand), nullptr, GL_DYNAMIC_DRAW);

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
    PROFILE_FUNCTION_GPU();
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
    PROFILE_FUNCTION_GPU();
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
    PROFILE_FUNCTION_GPU();
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


// numComponents is number of instances
static void EnableInstancing(
    u32 VAO, 
    void* instanceDataBuffer,
    u32 stride, u32 numElements,
    u32& vertexAttributeLocation, 
    u32& instanceVBO)
{
    PROFILE_FUNCTION();
    if (instanceVBO != 0) 
    {
        LOG_WARN("Attempted to enable instancing on a VBO that already has data");
        return;
    }
    glBindVertexArray(VAO);
    glGenBuffers(1, &instanceVBO);
    // when we call ConfigureVertexAttrib with this instanceVBO bound, this all gets bound up into the above VAO
    // thats how the VAO knows about our instance vbo
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // instance vbo 
    glBufferData(GL_ARRAY_BUFFER, stride*numElements, instanceDataBuffer, GL_STATIC_DRAW);
    // set up vertex attribute(s) for instance-specific data
    // if we want float/vec2/vec3/vec4 its not a big deal,
    // just send that into a vertex attribute as normal
    // but if its something like a mat4, it'll take up multiple
    // vertex attribute slots since we can only pass a max of 4 floats in one attribute
    // TODO: verify this works for all data types (float, vec2/3/4, mat2/3/4)
    u32 numFloatsInComponent = stride / 4;
    u64 numVec4sInComponent = std::max(1u, numFloatsInComponent / 4);
    for (u64 i = 0; i < numVec4sInComponent; i++) 
    {
        ConfigureVertexAttrib( // instance data
            vertexAttributeLocation+i, numFloatsInComponent / 4, GL_FLOAT, false, stride, (void*)(i*numFloatsInComponent));
        // update vertex attribute on every new instance of the mesh, not on every vertex (1 is a magic opengl number corresponding to "update-per-instance", rather than update-per-vertex)
        glVertexAttribDivisor(vertexAttributeLocation+i, 1);  
    }
    vertexAttributeLocation += numVec4sInComponent;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void GetDrawData(RMesh* meshes, u32 numMeshes, DrawElementsIndirectCommand* dst, u64& verticesMemSize, u64& indicesMemSize)
{
    size_t currentVertOffset = 0;
    size_t instanceOffset = 0;
    for (u32 i = 0; i < numMeshes; i++)
    {
        const RMesh& mesh = meshes[i];
        u32 numVertices = mesh.vertices.size / mesh.vertices.stride();

        DrawElementsIndirectCommand& cmd = dst[i];           
        cmd.count = mesh.indices.size / mesh.indices.stride(); // elements to render is the number of indices;
        cmd.instanceCount = Math::Max(mesh.numInstances, 1u);
        cmd.firstIndex = indicesMemSize;
        cmd.baseVertex = currentVertOffset;
        cmd.baseInstance = instanceOffset;

        verticesMemSize += mesh.vertices.size;
        indicesMemSize += mesh.indices.size;
        currentVertOffset += numVertices;
        instanceOffset += mesh.numInstances;
    }
}

void DrawModels(RendererData& renderer, Arena* arena)
{
    PROFILE_FUNCTION_GPU();
    // TODO: when we call PushEntity - increment some "generation" variable. Have another for the renderer data
    // if we call RendererDraw and the two generations are different, we know we've pushed a different set of entities and (possibly) need to regenerate our boofers
    Camera& cam = Camera::GetMainCamera();
    // each batch shares the *exact* same material, shader, and instance params/data

    for (auto& [batchHash, batch] : renderer.models)
    {
        if (!batch.active) continue;
        // collect data about this batch
        u32 numMeshes = batch.meshes.size;
        TINY_ASSERT(numMeshes <= MAX_NUM_MESHES_PER_BATCH && numMeshes > 0);
        DrawElementsIndirectCommand* drawCommands = arena_alloc_type(arena, DrawElementsIndirectCommand, numMeshes);
        TMEMSET(drawCommands, 0, sizeof(DrawElementsIndirectCommand) * numMeshes);
        u64 verticesMemSize, indicesMemSize = 0;
        GetDrawData(batch.meshes.get_elements(), numMeshes, drawCommands, verticesMemSize, indicesMemSize);
        // we need to allocate gpu buffers if the number of meshes in our batch increases
        bool shouldRegenerateBuffers = 
            batch.batchVAO == 0 || batch.verticesMemSize < verticesMemSize || batch.indicesMemSize < indicesMemSize;
        batch.verticesMemSize = verticesMemSize;
        batch.indicesMemSize = indicesMemSize;
        u32& VAO = batch.batchVAO;
        u32& VBO = batch.batchVBO;
        u32& EBO = batch.batchEBO;
        u32& instanceVBO = batch.instanceData.instanceVBO;
        if (shouldRegenerateBuffers)
        {
            batch.dirty = true;
            LOG_INFO("allocating %f vertices mb  and %f indices kb", (f64)verticesMemSize / 1000.0 / 1000.0, (f64)indicesMemSize / 1000.0);
            if (VAO != 0 || VBO != 0 || EBO != 0 || instanceVBO != 0)
            {
                // delete buffers
                glDeleteBuffers(1, &VAO);
                glDeleteBuffers(1, &VBO);
                glDeleteBuffers(1, &EBO);
                glDeleteBuffers(1, &instanceVBO);
                VAO = 0;
                VBO = 0;
                EBO = 0;
                instanceVBO = 0;
            }
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
            u32 vertexAttributeLocation = 0;
            ConfigureMeshVertexAttributes(vertexAttributeLocation);
            if (batch.isInstanced)
            {
                EnableInstancing(
                    VAO, 
                    batch.instanceData.instanceData, 
                    batch.instanceData.stride, 
                    batch.instanceData.numInstances, 
                    vertexAttributeLocation, 
                    instanceVBO);
            }
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
                size_t vertOffsetBytes = drawCommands[i].baseVertex * sizeof(RMeshVertex);
                glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)vertOffsetBytes, mesh.vertices.size, mesh.vertices.data);
                if (EBO != 0)
                {
                    u32 idxOffsetBytes = drawCommands[i].firstIndex;
                    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)idxOffsetBytes, mesh.indices.size, mesh.indices.data);
                }
            }
            batch.dirty = false;
        }
        const Shader& shader = batch.shader;
        const Material& material = batch.material;
        SetLightingUniforms(shader);
        material.SetShaderUniforms(shader);
        shader.use();
        glBindVertexArray(VAO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirectGPUBuffer);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, numMeshes * sizeof(DrawElementsIndirectCommand), drawCommands);
        //for (u32 i = 0; i < numMeshes; i++)
        //{
        //    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, numMeshes,
        //        GL_UNSIGNED_INT, (void*)drawCommands[i].firstIndex, drawCommands[i].instanceCount, drawCommands[i].baseVertex, drawCommands[i].baseInstance
        //    );
        //}
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, numMeshes, sizeof(DrawElementsIndirectCommand));
        arena_pop_latest(arena, drawCommands);
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

void PushModel(const Model& model, const Shader& shader)
{
    RendererData& renderer = GetRenderer();
    for (u32 i = 0; i < model.meshes.size(); i++)
    {
        const Mesh& mesh = model.meshes[i];
        if (!mesh.isVisible) continue;
        RMesh rmesh;
        rmesh.vertices = {const_cast<RMeshVertex*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(RMeshVertex)};
        rmesh.indices = {const_cast<RMeshIndex*>(mesh.indices.data()), mesh.indices.size() * sizeof(RMeshIndex)};
        rmesh.numInstances = mesh.instanceData.numInstances;
        // batches are bucketed by hashing their shader and material together
        u64 rmeshHash = MeshBatchHash(mesh, shader);
        MeshBatch& batch = renderer.models[rmeshHash];
        TINY_ASSERT(!batch.material.isValid() || batch.material == mesh.material); // either we are adding for the first time or they must be the same
        TINY_ASSERT(!batch.shader.isValid() || batch.shader == shader); // either we are adding for the first time or they must be the same
        batch.material = mesh.material;
        batch.shader = shader;
        batch.active = true;
        batch.isInstanced = mesh.instanceData.numInstances > 0;
        // instanced meshes that are being batched together *should* have the same pointer to the same instance data
        TINY_ASSERT(batch.instanceData.instanceData == nullptr || batch.instanceData.instanceData == mesh.instanceData.instanceData);
        batch.instanceData = mesh.instanceData;
        batch.meshes.push_back(rmesh);
    }
}

void PushEntity(const EntityRef& entity)
{
    EntityData& entityData = Entity::GetEntity(entity);
    const Model& model = entityData.model;
    const Shader& entityShader = model.cachedShader;
    PushModel(model, entityShader);
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