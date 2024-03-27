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

struct RMesh
{
    BufferView<Vertex> vertices = {};
    BufferView<u32> indices = {};
    Transform tf = {};
};

struct RendererData
{
    Arena* arena = {};
    FixedGrowableArray<RPoint, 300> points = {};
    u32 pointsVAO, pointsVBO = 0;
    FixedGrowableArray<RLine, 300> lines = {};
    u32 linesVAO, linesVBO = 0;
    FixedGrowableArray<RTriangle, 300> triangles = {};
    u32 trianglesVAO, trianglesVBO = 0;
};

namespace Renderer
{

static Shader defaultShapeShader;

void InitializeRenderer(Arena* arena)
{
    RendererData* rendererMem = arena_alloc_and_init<RendererData>(arena);
    GetEngineCtx().renderer = rendererMem;
    rendererMem->arena = arena;

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


void PushDebugRenderMarker(const char* name)
{
    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
}
void PopDebugRenderMarker()
{
    glPopDebugGroupKHR();
}

} // namespace Renderer