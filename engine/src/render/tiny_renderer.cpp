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
#include "tiny_fs.h"
#include "render/skybox.h"
#include "tiny_imgui.h"
#include "render/postprocess.h"
#include "physics/tiny_physics.h"
#include "res/shaders/shader_defines.glsl"


constexpr u32 MAX_NUM_RENDER_PASSES = 10;
constexpr u32 MAX_NUM_MESHES_PER_BATCH = 500; // arbitrary
constexpr u32 MAX_NUM_PRIMITIVE_DRAWS = 5000; // TODO: strechy gpu buffer

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
    FixedGrowableArray<RMesh, MAX_NUM_MESHES_PER_BATCH> meshes = {};
    u32 batchVAO, batchVBO, batchEBO = 0;
    u64 verticesMemSize, indicesMemSize = 0;
    GPUInstanceData instanceData = {};
    bool isInstanced = false; // instanced meshes in a batch have the same "instance data". (model matrices)
    bool dirty = true;
    // when pushing a mesh into a batch, this is set. It is set back to false when the batch is rendered.
    // additionally, the batch is ONLY rendered if this is set. 
    // With this, to draw a mesh every frame you must "push" that mesh to the renderer every frame (through a model or entity or whatever)
    bool active = true;
    Shader prepassShaders[MAX_NUM_RENDER_PASSES] = {};
};

void ClearMeshBatch(MeshBatch& batch)
{
    // making sure we do *not* clear the VAO/VBO data
    batch.meshes.clear();
    batch.material = {};
    batch.shader = {};
    batch.active = false;
}

// hash required data for a batch. Inputs that hash the same will be batched
static u64 MeshBatchHash(const Material& material, const Shader& shader, const GPUInstanceData& instanceData)
{
    u64 result = 0;
    // bottom 32 bits are material id, top 32 bits are shader id
    result |= MaterialHasher()(material);
    result |= ShaderHasher()(shader) << 32;
    if (instanceData.numInstances > 0)
    {
        // cannot batch instanced draws with non-instanced draws. Additionally instance data should be the same for two instanced meshes to be batched
        result ^= HashBytes((u8*)&instanceData, sizeof(instanceData)); 
    }
    result = HashBytesL((u8*)&result, sizeof(result));
    return result;
}

struct DrawElementsIndirectCommand 
{
    u32  count; // num indices
    u32  instanceCount;
    u32  firstIndex; // bytes
    s32  baseVertex; // vertex idx
    u32  baseInstance;
};

struct RenderPass;
// returns true to trigger a draw call. Returning false = don't draw the batch
// called for every batch in a render pass
typedef bool (*RenderPassPreDrawFunc)(const RenderPass&, const MeshBatch&, const Shader&);
// called once before any batches are rendered
typedef bool (*RenderPassPreProcessFunc)(const RenderPass&, u32 passIndex);
// called once after all batches in a pass are rendered
typedef void (*RenderPassPostProcessFunc)(const RenderPass&, u32 passIndex);
// called when initializing the renderpass
typedef void (*RenderPassInitialize)(RenderPass&);
struct RenderPass
{
    Framebuffer output = {};
    Framebuffer::FramebufferProperties outputProperties = {};
    // if only frag is specified, vertex will be pulled from the batch's shader (user-authored)
    const char* fragShader = nullptr;
    const char* vertShader = nullptr;
    Shader passShader = {};
    RenderPassPreDrawFunc preDrawFunc = nullptr;
    RenderPassPreProcessFunc preprocessFunc = nullptr;
    RenderPassPostProcessFunc postprocessFunc = nullptr;
    RenderPassInitialize initializeFunc = nullptr;
    #define RENDERPASS_MAX_NAME_LENGTH 30
    const char* passName = "Unnamed Pass";
    bool active = false;
    bool needsLightingMaterialUniforms = false;
};

struct RendererData
{
    Arena arena = {};
    FixedGrowableArray<RPoint, MAX_NUM_PRIMITIVE_DRAWS> points = {};
    u32 pointsVAO, pointsVBO = 0;
    FixedGrowableArray<RLine, MAX_NUM_PRIMITIVE_DRAWS> lines = {};
    u32 linesVAO, linesVBO = 0;
    FixedGrowableArray<RTriangle, MAX_NUM_PRIMITIVE_DRAWS> triangles = {};
    u32 trianglesVAO, trianglesVBO = 0;
    typedef std::unordered_map<u64, MeshBatch> BatchMap;
    BatchMap meshesToRender = {};
    u32 indirectGPUBuffer = 0;
    RenderPass outputPasses[MAX_NUM_RENDER_PASSES] = {};
    //Framebuffer finalOutput = {};
    Skybox skybox = {};
    bool needsSetup = true;
    s32 debugRenderPassVisIdx = -1;
    s32 debugRenderPassAttachmentIdx = 0;
};

RendererData& GetRenderer()
{
    return *GetEngineCtx().renderer;
}

enum PremadeRenderPassType
{
    SHADOWS,
    GBUFFER,
    SSAO,
    SSAO_BLUR,
    LIGHTING,

    NUM_PREMADE_RENDER_PASSES,
};

// should match the color attachments in the gbuffer render pass output properties
enum GbufferAttachments
{
    NORMS_DEPTH,
    ALBEDO_SPEC,
    OBJ_MAT_ID,
    POSITIONWS,

    NUM_GBUFFER_ATTACHMENTS,
};

void SetGbufferUniforms(const RenderPass& gbufferPass, const Shader& shader)
{
    Texture normsDepth = gbufferPass.output.GetColorTexture(GbufferAttachments::NORMS_DEPTH);
    shader.TryAddSampler(normsDepth, "depthNormals");
    Texture albedoSpec = gbufferPass.output.GetColorTexture(GbufferAttachments::ALBEDO_SPEC);
    shader.TryAddSampler(albedoSpec, "albedoSpec");
    Texture objMatId = gbufferPass.output.GetColorTexture(GbufferAttachments::OBJ_MAT_ID);
    shader.TryAddSampler(objMatId, "objMatId");
    Texture gbuf_positionWS = gbufferPass.output.GetColorTexture(GbufferAttachments::POSITIONWS);
    shader.TryAddSampler(gbuf_positionWS, "gbuf_positionWS");
}

bool PrepassSetUniformsDirectionalShadows(
    const RenderPass& pass, 
    const MeshBatch& batch, 
    const Shader& shader)
{
    shader.setUniform("isDirectionalShadowPass", true);
    return true;
}


void PrepassShadowsPostprocess(
    const RenderPass& pass,
    u32 passIndex)
{
    RendererData& renderer = GetRenderer();
    GetEngineCtx().lightsSubsystem->directionalShadowMap = pass.output;
}

void SSAOPass(
    const RenderPass& pass,
    u32 passIndex)
{
    RendererData& renderer = GetRenderer();
    const Framebuffer& gbuffer = renderer.outputPasses[PremadeRenderPassType::GBUFFER].output;
    if (gbuffer.isValid())
    {
        Postprocess::PostprocessFramebuffer(gbuffer, pass.output, pass.passShader);
    }
}

void SSAOInit(RenderPass& pass)
{
    RendererData& renderer = GetRenderer();
    pass.passShader = Shader(ResPath("shaders/default_sprite.vert"), ResPath("shaders/ao.frag"));
    Postprocess::ApplySSAOUniforms(pass.passShader);
    Framebuffer* depthnorms = &renderer.outputPasses[PremadeRenderPassType::GBUFFER].output;
    pass.passShader.TryAddSampler(depthnorms->GetColorTexture(GbufferAttachments::NORMS_DEPTH), "depthNormals");
}

void SSAOBlurPass(
    const RenderPass& pass,
    u32 passIndex)
{
    RendererData& renderer = GetRenderer();
    const Framebuffer& ssao = renderer.outputPasses[PremadeRenderPassType::SSAO].output;
    if (ssao.isValid())
    {
        Postprocess::PostprocessFramebuffer(ssao, pass.output, pass.passShader);
    }
}

void SSAOBlurInit(RenderPass& pass)
{
    pass.passShader = Shader(ResPath("shaders/default_sprite.vert"), ResPath("shaders/simple_blur.frag"));
}

void LightingPassInit(RenderPass& pass)
{
    // "lighting" is really just a postprocessing effect, so we draw it as a sprite
    pass.passShader = Shader(ResPath("shaders/default_sprite.vert"), ResPath("shaders/basic_lighting.frag"));
}

void LightingPassPostProcess(
    const RenderPass& pass,
    u32 passIndex)
{
    RendererData& renderer = GetRenderer();
    const Shader& shader = pass.passShader;
    const RenderPass& gbufferPass = renderer.outputPasses[PremadeRenderPassType::GBUFFER];
    SetGbufferUniforms(gbufferPass, shader);
    const Framebuffer& ssaoOutput = renderer.outputPasses[PremadeRenderPassType::SSAO_BLUR].output; 
    shader.TryAddSampler(ssaoOutput.GetColorTexture(0), "aoTexture");
    shader.TryAddSampler(renderer.skybox.cubemap, "skybox");
    SetLightingUniforms(shader);
    Framebuffer::Blit(&gbufferPass.output, &pass.output, true); // need depth in our lighting framebuffer (should we blit stencil too?)
    Postprocess::PostprocessFramebufferInPlace(pass.output, shader);
}

bool GbufferPreprocess(
    const RenderPass& pass, 
    u32 passIndex)
{
    // don't blend during gbuffer pass since we're just outputting raw values to our color attachments
    glDisable(GL_BLEND);
    return true;
}
void GbufferPostprocess(
    const RenderPass& pass,
    u32 passIndex)
{
    glEnable(GL_BLEND);
}

// meant to be used in a render pass preProcess, returning false indicates that we should skip drawing this pass
bool NoDraw(
    const RenderPass& pass, 
    u32 passIndex)
{
    return false;
}

static RenderPass premadeRenderPrepasses[] = 
{
    {
        .outputProperties = 
        {
            .size = glm::vec2(2048),
            .numColorAttachments = 0,
            .hasDepth = true,
        },
        .fragShader = "shaders/depth.frag",
        .preDrawFunc = PrepassSetUniformsDirectionalShadows,
        .postprocessFunc = PrepassShadowsPostprocess,
        .passName = "Directional Shadows",
    },
    {
        .outputProperties = 
        {
            // should match GbufferAttachments
            .colorAttachments = 
            {
                {
                    .internalFormat = Framebuffer::FramebufferProperties::FramebufferInternalFormat::RGBA16F,
                    .format = Framebuffer::FramebufferProperties::FramebufferFormat::RGBA,
                    .dataType = Framebuffer::FramebufferProperties::PixelDataType::FLOAT,
                    .attachmentName = "Depth & Normals",
                },
                {
                    .internalFormat = Framebuffer::FramebufferProperties::FramebufferInternalFormat::RGBA16F,
                    .format = Framebuffer::FramebufferProperties::FramebufferFormat::RGBA,
                    .dataType = Framebuffer::FramebufferProperties::PixelDataType::FLOAT,
                    .attachmentName = "Albedo & Specular",
                },
                {
                    // TODO: can't visualize this attachment b/c can't sample an integer texture with a normal sampler2d
                    .internalFormat = Framebuffer::FramebufferProperties::FramebufferInternalFormat::RG32UI,
                    .format = Framebuffer::FramebufferProperties::FramebufferFormat::RG_INTEGER,
                    .dataType = Framebuffer::FramebufferProperties::PixelDataType::UNSIGNED_INT,
                    .attachmentName = "Object & Material ID",
                },
                {
                    // TODO: can't visualize this attachment b/c can't sample an integer texture with a normal sampler2d
                    .internalFormat = Framebuffer::FramebufferProperties::FramebufferInternalFormat::RGB16F,
                    .format = Framebuffer::FramebufferProperties::FramebufferFormat::RGB,
                    .dataType = Framebuffer::FramebufferProperties::PixelDataType::FLOAT,
                    .attachmentName = "Worldspace Position",
                },
            },
            .numColorAttachments = GbufferAttachments::NUM_GBUFFER_ATTACHMENTS,
        },
        .fragShader = "shaders/prepass.frag",
        .preprocessFunc = GbufferPreprocess,
        .postprocessFunc = GbufferPostprocess,
        .passName = "Gbuffer",
        .needsLightingMaterialUniforms = true,
    },
    { // SSAO needs to be after depth&normals (FUTURE: rendergraph)
        .preprocessFunc = NoDraw, // don't draw any geometry, we're just postprocessing the depth&norms framebuffer
        .postprocessFunc = SSAOPass,
        .initializeFunc = SSAOInit,
        .passName = "SSAO",
    },
    {
        .preprocessFunc = NoDraw,
        .postprocessFunc = SSAOBlurPass,
        .initializeFunc = SSAOBlurInit,
        .passName = "SSAO Blur",
    },
    {
        // lighting renderpass
        .preprocessFunc = NoDraw,
        .postprocessFunc = LightingPassPostProcess,
        .initializeFunc = LightingPassInit,
        .passName = "Lighting",
    },
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

static void DrawPoints(RendererData& renderer, f32 pointSize)
{
    PROFILE_FUNCTION_GPU();
    u32 numPoints = renderer.points.size;
    RPoint* points = renderer.points.get_elements();
    if (numPoints == 0) return;
    TINY_ASSERT(numPoints < MAX_NUM_PRIMITIVE_DRAWS);
    u32& VAO = renderer.pointsVAO;
    u32& VBO = renderer.pointsVBO;
    if (VAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RPoint) * MAX_NUM_PRIMITIVE_DRAWS, points, GL_DYNAMIC_DRAW));
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
    TINY_ASSERT(numLines < MAX_NUM_PRIMITIVE_DRAWS);
    u32& VAO = renderer.linesVAO;
    u32& VBO = renderer.linesVBO;
    if (VAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RLine) * MAX_NUM_PRIMITIVE_DRAWS, lines, GL_DYNAMIC_DRAW));
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
    TINY_ASSERT(numTriangles < MAX_NUM_PRIMITIVE_DRAWS);
    u32& quadVAO = renderer.trianglesVAO;
    u32& VBO = renderer.trianglesVBO;
    if (quadVAO == 0) 
    {
        GLCall(glGenVertexArrays(1, &quadVAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(RTriangle) * MAX_NUM_PRIMITIVE_DRAWS, triangles, GL_DYNAMIC_DRAW));
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
void posterizationEffectImGui()
{
    if (ImGui::CollapsingHeader("Posterization Effect"))
    {
        Shader* postprocessShader = Postprocess::GetPostprocessingShader();
        if (!postprocessShader->isValid()) return;

        #define floatimgui(varname, f_default) \
            static float varname = f_default; \
            ImGui::DragFloat(#varname, &varname, 0.01f); \
            postprocessShader->setUniform(#varname, varname);
        #define colorimgui(varname, v_default) \
            static glm::vec3 varname = v_default; \
            ImGui::ColorEdit3(#varname, &varname.x); \
            postprocessShader->setUniform(#varname, varname);

        floatimgui(palette_scene_weight, 0.0);
        floatimgui(palette_hue, 5.1);
        floatimgui(palette_sat, 0.64);
        floatimgui(palette_val, 8.8);
        floatimgui(palette_brightness, 1.6);
        floatimgui(palette_darkness, 0.51);
        floatimgui(palette_tint, 0.18);
        floatimgui(palette_saturation, 0.24);
        colorimgui(palette_base, glm::vec3(1));
    }
}

void DebugPreDraw()
{
    posterizationEffectImGui();
    // collision shapes
    PhysicsDebugRender();
    RendererData& renderer = GetRenderer();
    if (ImGui::CollapsingHeader("Render passes"))
    {
        ImGui::SliderInt("Color Attachment Idx", &renderer.debugRenderPassAttachmentIdx, 0, MAX_NUM_COLOR_ATTACHMENTS);
        u32 numRenderpassNames;
        const char** renderpassNames = Renderer::GetRenderPassNames(GetFrameAllocator(), numRenderpassNames);
        for (u32 i = 0; i < numRenderpassNames; i++)
        {
            if (ImGui::RadioButton(renderpassNames[i], renderer.debugRenderPassVisIdx == i))
            {
                renderer.debugRenderPassVisIdx = renderer.debugRenderPassVisIdx == i ? -1 : i;
            }
        }
    }
    // red is x, green is y, blue is z
    // should put this on the screen in the corner permanently
    f32 axisGizmoScale = 0.03f;
    glm::vec3 camFront = Camera::GetMainCamera().cameraFront;
    glm::vec3 camUp = Camera::GetMainCamera().cameraUp;
    glm::vec3 cameraRight = glm::normalize(glm::cross(camFront, camUp));
    camUp = glm::normalize(glm::cross(camFront, cameraRight));
    ImGui::Text("cam front: %f %f %f", camFront.x, camFront.y, camFront.z);
    ImGui::Text("cam up: %f %f %f", camUp.x, camUp.y, camUp.z);
    ImGui::Text("cam right: %f %f %f", cameraRight.x, cameraRight.y, cameraRight.z);
    glm::vec3 camRel = Camera::GetMainCamera().cameraPos + camFront;
    Renderer::PushLine(camRel, glm::vec3(axisGizmoScale,0,0) + camRel, {1,0,0,1});
    Renderer::PushLine(camRel, glm::vec3(0,axisGizmoScale,0) + camRel, {0,1,0,1});
    Renderer::PushLine(camRel, glm::vec3(0,0,axisGizmoScale) + camRel, {0,0,1,1});
    // lights visualization
    for (LightPoint& pointLight : GetEngineCtx().lightsSubsystem->lights.pointLights)
    {
        pointLight.Visualize();
    }
    GetEngineCtx().lightsSubsystem->lights.sunlight.Visualize();
}


// numComponents is number of instances
void EnableInstancing(
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
    glBufferData(GL_ARRAY_BUFFER, stride*numElements, instanceDataBuffer, GL_DYNAMIC_DRAW);
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
    size_t indexOffset = 0;
    size_t instanceOffset = 0;
    for (u32 i = 0; i < numMeshes; i++)
    {
        const RMesh& mesh = meshes[i];
        u32 numVertices = mesh.vertices.size / mesh.vertices.stride();
        u32 numIndices = mesh.indices.size / mesh.indices.stride();

        DrawElementsIndirectCommand& cmd = dst[i];           
        cmd.count = numIndices; 
        cmd.instanceCount = Math::Max(mesh.numInstances, 1u);
        cmd.firstIndex = indexOffset;
        cmd.baseVertex = currentVertOffset;
        cmd.baseInstance = instanceOffset;

        verticesMemSize += mesh.vertices.size;
        indicesMemSize += mesh.indices.size;
        currentVertOffset += numVertices;
        indexOffset += numIndices;
        instanceOffset += mesh.numInstances;
    }
}

static void CheckRegenerateGPUBuffers(
    MeshBatch& batch, 
    DrawElementsIndirectCommand* drawCommands, 
    u64 verticesMemSize, 
    u64 indicesMemSize)
{
    // collect data about this batch
    u32 numMeshes = batch.meshes.size;
    TINY_ASSERT(numMeshes <= MAX_NUM_MESHES_PER_BATCH && numMeshes > 0);
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
        glBufferData(GL_ARRAY_BUFFER, verticesMemSize, NULL, GL_DYNAMIC_DRAW);
        if (indicesMemSize > 0)
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesMemSize, NULL, GL_DYNAMIC_DRAW);
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
            const DrawElementsIndirectCommand& drawCmd = drawCommands[i];
            size_t vertOffsetBytes = drawCmd.baseVertex * sizeof(RMeshVertex);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)vertOffsetBytes, mesh.vertices.size, mesh.vertices.data);
            if (EBO != 0)
            {
                u32 idxOffsetBytes = drawCmd.firstIndex * sizeof(RMeshIndex);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)idxOffsetBytes, mesh.indices.size, mesh.indices.data);
            }
        }
        batch.dirty = false;
    }
}

// if shader is blank, we use batch shader. Shader can be overridden for use with prepasses
void DrawBatch(
    const RendererData& renderer,
    Arena* arena,
    MeshBatch& batch, 
    const RenderPass& pass = {},
    const Shader& shaderOverride = {})
{
    PROFILE_FUNCTION_GPU();
    Shader selectedShader = batch.shader;
    if (shaderOverride.isValid())
    {
        selectedShader = shaderOverride;
    }
    bool shouldDraw = true;
    if (pass.preDrawFunc)
    {
        PROFILE_GPU_SCOPE("Predraw");
        // the predraw function can dictate if we should draw the batch or not.
        // this is to allow for render passes to do some manual processing and not trigger a draw (I.E. postprocessing of other render passes)
        shouldDraw = pass.preDrawFunc(pass, batch, selectedShader);
    }
    if (shouldDraw)
    {
        // prep draw data
        u32 numMeshes = batch.meshes.size;
        if (numMeshes < 1) return;
        DrawElementsIndirectCommand* drawCommands = arena_alloc_type(arena, DrawElementsIndirectCommand, numMeshes);
        TMEMSET(drawCommands, 0, sizeof(DrawElementsIndirectCommand) * numMeshes);
        u64 verticesMemSize = 0;
        u64 indicesMemSize = 0;
        GetDrawData(batch.meshes.get_elements(), numMeshes, drawCommands, verticesMemSize, indicesMemSize);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirectGPUBuffer);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, numMeshes * sizeof(DrawElementsIndirectCommand), drawCommands);
        arena_pop_latest(arena, drawCommands);
        // use shader & manage automatic uniforms
        if (batch.shader != selectedShader)
        {
            TransferUniforms(batch.shader, selectedShader, true);
        }
        if (pass.needsLightingMaterialUniforms)
        {
            // don't need lighting/material stuff for some passes
            SetLightingUniforms(selectedShader);
            const Material& material = batch.material;
            material.SetShaderUniforms(selectedShader); 
        }
        // for prepasses, where we pass in a valid shaderOverride,
        // the selectedShader will not be the same as the batch's shader.
        // the batch shader's uniforms will be transferred to the prepass shader
        // this is to facilitate custom vertex shaders in conjunction with this automatic prepass system
        selectedShader.use();
        glBindVertexArray(batch.batchVAO);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, numMeshes, sizeof(DrawElementsIndirectCommand));
    }
}

void BatchPreprocessing(
    RendererData& renderer, 
    Arena* arena)
{
    // Checking if we need to resize GPU buffers, compiling prepass shaders if need be....
    for (auto& [batchHash, batch] : renderer.meshesToRender)
    {
        if (!batch.active) continue;
        // collect data about this batch
        u32 numMeshes = batch.meshes.size;
        TINY_ASSERT(numMeshes <= MAX_NUM_MESHES_PER_BATCH && numMeshes > 0);
        // get draw commands and check if we need to resize gpu buffers
        DrawElementsIndirectCommand* drawCommands = arena_alloc_type(arena, DrawElementsIndirectCommand, numMeshes);
        TMEMSET(drawCommands, 0, sizeof(DrawElementsIndirectCommand) * numMeshes);
        u64 verticesMemSize = 0;
        u64 indicesMemSize = 0;
        GetDrawData(batch.meshes.get_elements(), numMeshes, drawCommands, verticesMemSize, indicesMemSize);
        CheckRegenerateGPUBuffers(batch, drawCommands, verticesMemSize, indicesMemSize);
        arena_pop_latest(arena, drawCommands);
    }
}

void DrawScene(RendererData& renderer, Arena* arena)
{
    PROFILE_FUNCTION();
    // TODO: when we call PushEntity - increment some "generation" variable. Have another for the renderer data
    // if we call RendererDraw and the two generations are different, we know we've pushed a different set of entities and (possibly) need to regenerate our boofers
    // each batch shares the *exact* same material, shader, and instance params/data
    BatchPreprocessing(renderer, arena);

    // render passes
    u32 passIndex = 0;
    for (;passIndex < MAX_NUM_RENDER_PASSES; passIndex++)
    {
        PROFILE_GPU_SCOPE("Render pass");
        RenderPass& pass = renderer.outputPasses[passIndex];
        if (!pass.output.isValid() || !pass.active) continue;
        Renderer::PushDebugRenderMarker(TextFormat("Render pass %i", passIndex));
        bool shouldDraw = true;
        if (pass.preprocessFunc)
        {
            PROFILE_GPU_SCOPE("Preprocess");
            shouldDraw = pass.preprocessFunc(pass, passIndex);
        }
        if (shouldDraw)
        {
            pass.output.Bind();
            ClearGLBuffers();
            for (auto& [batchHash, batch] : renderer.meshesToRender)
            {
                DrawBatch(renderer, arena, batch, pass, batch.prepassShaders[passIndex]);
            }
        }
        if (pass.postprocessFunc)
        {
            PROFILE_GPU_SCOPE("Postprocess");
            pass.postprocessFunc(pass, passIndex);
        }
        Renderer::PopDebugRenderMarker();
    }

    for (auto& [batchHash, batch] :  renderer.meshesToRender)
    {
        ClearMeshBatch(batch);
    }

    //renderer.finalOutput.Bind();
    // basic shape drawing
    Renderer::PushDebugRenderMarker("Points");
    DrawPoints(renderer, 10.0f);
    renderer.points.clear();
    Renderer::PopDebugRenderMarker();
    Renderer::PushDebugRenderMarker("Lines");
    DrawLines(renderer);
    renderer.lines.clear();
    Renderer::PopDebugRenderMarker();
    Renderer::PushDebugRenderMarker("Triangles");
    DrawTriangles(renderer);
    renderer.triangles.clear();
    Renderer::PopDebugRenderMarker();

    renderer.skybox.Draw();
}

glm::vec2 GetRendererDimensions()
{
    // .... is this what we want?
    return Camera::GetScreenDimensions();
}

void SetupRenderPasses(
    RendererData& renderer)
{
    renderer.skybox = Skybox();
    glm::vec2 outputFramebufferDimensions = GetRendererDimensions();
    //renderer.finalOutput = Framebuffer(outputFramebufferDimensions.x, outputFramebufferDimensions.y);
    for (u32 i = 0; i < ARRAY_SIZE(premadeRenderPrepasses); i++)
    {
        RenderPass& renderPass = renderer.outputPasses[i];
        renderPass = premadeRenderPrepasses[i];
        Framebuffer::FramebufferProperties* fbProperties = &premadeRenderPrepasses[i].outputProperties;
        if (fbProperties->size.x == 0 || fbProperties->size.y == 0)
        {
            fbProperties->size = outputFramebufferDimensions;
        }
        renderPass.output = Framebuffer(fbProperties);
        renderPass.active = true;
        if (renderPass.initializeFunc)
        {
            renderPass.initializeFunc(renderPass);
        }
    }
    for (u32 i = 0; i < ARRAY_SIZE(premadeRenderPrepasses); i++)
    {
        RenderPass& premadePass = premadeRenderPrepasses[i];
        // some passes may not have frag shaders
        if (premadePass.fragShader)
        {
            for (auto& [batchHash, batch] : renderer.meshesToRender)
            {
                const Shader& batchShader = batch.shader;
                // set up premade render passes (shadows, depth/norms, postprocessing, etc)
                ShaderLocation batchShaderLocations = GetShaderPaths(batchShader);
                // for each batch's shader, we need variations of it for our prepasses.
                // shaders with custom vertex shaders will keep their behavior during prepasses and have their fragment shaders overridden
                Shader prepassShader = Shader(batchShaderLocations.first, ResPath(premadePass.fragShader));
                batch.prepassShaders[i] = prepassShader;
            }
        }
    }
    Shader postprocessingShader = Shader(ResPath("shaders/default_sprite.vert"), ResPath("shaders/postprocess.frag"));
    Postprocess::SetPostprocessShader(postprocessingShader);
}

void SetDebugOutputRenderPass(u32 renderpassIdx)
{
    RendererData& renderer = GetRenderer();
    renderpassIdx = Math::Clamp(renderpassIdx, 0u, MAX_NUM_RENDER_PASSES);
    renderer.debugRenderPassVisIdx = renderpassIdx;
}

const char** GetRenderPassNames(Arena* arena, u32& numNames)
{
    numNames = 0;
    RendererData& renderer = GetRenderer();
    const char** result = (const char**)arena_alloc(arena, MAX_NUM_RENDER_PASSES * sizeof(char*));
    for (u32 i = 0; i < MAX_NUM_RENDER_PASSES; i++)
    {
        if (renderer.outputPasses[i].active && renderer.outputPasses[i].output.isValid())
        {
            const char* name = renderer.outputPasses[i].passName;
            const char* nameDst = (const char*)arena_alloc(arena, RENDERPASS_MAX_NAME_LENGTH);
            TMEMCPY((void*)nameDst, (void*)name, RENDERPASS_MAX_NAME_LENGTH); // NOTE: this touches memory outside the string   :/
            result[numNames++] = nameDst;
        }
    }
    return result;
}

Framebuffer* RendererDraw()
{
    PROFILE_FUNCTION();
    PROFILE_FUNCTION_GPU();
    DebugPreDraw();
    RendererData& renderer = GetRenderer();
    Renderer::PushDebugRenderMarker("debug render");
    Renderer::PopDebugRenderMarker();
    if (renderer.needsSetup)
    {
        SetupRenderPasses(renderer);
        renderer.needsSetup = false;
    }
    ShaderSystemPreDraw(); 
    ClearGLBuffers();
    DrawScene(renderer, &renderer.arena);

    Renderer::PushDebugRenderMarker("Postprocessing");
    Framebuffer* framebuffer = &renderer.outputPasses[PremadeRenderPassType::NUM_PREMADE_RENDER_PASSES-1].output;
    Shader* postprocessShader = Postprocess::GetPostprocessingShader();
    if (postprocessShader->isValid())
    {
        Postprocess::PostprocessFramebufferInPlace(*framebuffer, *postprocessShader);
    }
    Renderer::PopDebugRenderMarker();
    glm::vec2 scrn = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
    Transform2D debugRenderTf = Transform2D(glm::vec2(0), scrn);
    if (renderer.debugRenderPassVisIdx != -1)
    {
        framebuffer->Bind();
        glClear(GL_DEPTH_BUFFER_BIT); // get rid of (depth of) final render so we can draw debug stuff on top
        RenderPass& pass = renderer.outputPasses[renderer.debugRenderPassVisIdx];
        Framebuffer& output = pass.output;
        // prioritize depth, this typically is only for shadow maps
        Texture depth = output.GetDepthTexture();
        // since we store random stuff in the alpha channel sometimes (I.E. gbuffer attachments) we don't want to do blending for this debug vis
        glDisable(GL_BLEND);
        if (depth.isValid())
        {
            output.DrawToFramebuffer(
                    *framebuffer, 
                    debugRenderTf, 
                    FramebufferAttachmentType::DEPTH, 
                    {});
        }
        else if (output.isValid()) 
        {
            output.DrawToFramebuffer(
                        *framebuffer, 
                        debugRenderTf, 
                        (FramebufferAttachmentType)(FramebufferAttachmentType::COLOR0 + renderer.debugRenderPassAttachmentIdx), 
                        {});
        }
        glEnable(GL_BLEND);
    }
    
    return framebuffer;
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

void PushFrustum(const glm::mat4& projection, const glm::mat4& view, glm::vec4 color)
{
    glm::mat4 m = glm::inverse(projection * view);
    static glm::vec3 _cameraFrustumCornerVertices[8] =
    {
        // near
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1},  {-1,  1, -1},
        // far
        {-1, -1, 1},	{ 1, -1, 1},	{ 1,  1, 1},  {-1,  1, 1}
    };
    static u32 frustumIndices[] = {0,1 , 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7};
    glm::vec3 camFrustumVerts[8];
    for (u32 i = 0; i < 8; i++)
    {
        glm::vec3 defaultVert = _cameraFrustumCornerVertices[i];
        glm::vec4 vert = m * glm::vec4(defaultVert, 1.0);
        camFrustumVerts[i] = glm::vec3(vert) / vert.w;
    }
    for (u32 i = 0; i < ARRAY_SIZE(frustumIndices); i+=2)
    {
        Renderer::PushLine(camFrustumVerts[frustumIndices[i]], camFrustumVerts[frustumIndices[i+1]], color);
    }
}

static void AddToBatch(
    const RMesh& mesh, 
    const Shader& shader, 
    const Material& material,
    const GPUInstanceData& instanceData)
{
    RendererData& renderer = GetRenderer();
    // batches are bucketed by hashing properties of their mesh and their shader
    u64 rmeshHash = MeshBatchHash(material, shader, instanceData);
    MeshBatch& batch = renderer.meshesToRender[rmeshHash];
    TINY_ASSERT(!batch.material.isValid() || batch.material == material); // either we are adding for the first time or they must be the same
    TINY_ASSERT(!batch.shader.isValid() || batch.shader == shader); // either we are adding for the first time or they must be the same
    batch.material = material;
    batch.shader = shader;
    batch.active = true;
    batch.isInstanced = instanceData.numInstances > 0;
    // instanced meshes that are being batched together *should* have the same pointer to the same instance data
    TINY_ASSERT(batch.instanceData.instanceData == nullptr || batch.instanceData.instanceData == instanceData.instanceData);
    batch.instanceData = instanceData;
    batch.meshes.push_back(mesh);
}

void PushModel(const Model& model, const Shader& shader)
{
    for (u32 i = 0; i < model.meshes.size(); i++)
    {
        const Mesh& mesh = model.meshes[i];
        if (!mesh.isVisible) continue;
        RMesh rmesh;
        static_assert(sizeof(RMeshVertex) == sizeof(Vertex));
        static_assert(sizeof(RMeshIndex) == sizeof(u32));
        rmesh.vertices = {const_cast<RMeshVertex*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(RMeshVertex)};
        rmesh.indices = {const_cast<RMeshIndex*>(mesh.indices.data()), mesh.indices.size() * sizeof(RMeshIndex)};
        rmesh.numInstances = mesh.instanceData.numInstances;
        AddToBatch(rmesh, shader, mesh.material, mesh.instanceData);
    }
}

void PushEntity(const EntityRef& entity)
{
    EntityData& entityData = Entity::GetEntity(entity);
    if (Entity::IsFlag(entityData, EntityFlags::DISABLED)) return;
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