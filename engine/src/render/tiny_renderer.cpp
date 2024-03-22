#include "tiny_renderer.h"

#include "mem/tiny_arena.h"
#include "containers/fixed_growable_array.h"
#include "tiny_engine.h"
#include "render/shader.h"
#include "render/tiny_ogl.h"
#include "camera.h"
#include "tiny_profiler.h"

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

constexpr u32 NUM_INITIAL_ALLOCATED_RENDERABLES = 100;
struct RendererData
{
    FixedGrowableArray<RPoint, NUM_INITIAL_ALLOCATED_RENDERABLES> points = {};
    FixedGrowableArray<RLine, NUM_INITIAL_ALLOCATED_RENDERABLES> lines = {};
    FixedGrowableArray<RTriangle, NUM_INITIAL_ALLOCATED_RENDERABLES> triangles = {};
};

namespace Renderer
{

void InitializeRenderer(Arena* arena)
{
    RendererData* rendererMem = (RendererData*)arena_alloc(arena, sizeof(RendererData));
    GetEngineCtx().renderer = rendererMem;
    rendererMem->points = FixedGrowableArray<RPoint, NUM_INITIAL_ALLOCATED_RENDERABLES>();
    rendererMem->lines = FixedGrowableArray<RLine, NUM_INITIAL_ALLOCATED_RENDERABLES>();
    rendererMem->triangles = FixedGrowableArray<RTriangle, NUM_INITIAL_ALLOCATED_RENDERABLES>();
}

RendererData& GetRenderer()
{
    return *GetEngineCtx().renderer;
}

static void DrawPoints(RPoint* points, u32 numPoints, f32 pointSize)
{
    if (numPoints == 0) return;
    static Shader shader;
    if (!shader.isValid()) 
    {
        shader = Shader::CreateShaderFromStr(
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
out vec4 FragColor;
in vec4 color;
void main(){
	FragColor = color;
}
)"
        );
    }
    static u32 VAO = 0;
    static u32 VBO = 0;
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
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GLCall(glBindVertexArray(0));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RPoint) * numPoints, points));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1);
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.use();
    GLCall(glBindVertexArray(VAO));
    glPointSize(pointSize);
    GLCall(glDrawArrays(GL_POINTS, 0, numPoints));
    GLCall(glBindVertexArray(0));
}

void DrawLines(RLine* lines, u32 numLines)
{
    if (numLines == 0) return;
    static Shader shader;
    if (!shader.isValid()) 
    {
        shader = Shader::CreateShaderFromStr(
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
 
out vec4 FragColor;
in vec4 color;
void main(){
	FragColor = color;
}
)"
        );
    }
    static u32 VAO = 0;
    static u32 VBO = 0;
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
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GLCall(glBindVertexArray(0));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RLine) * numLines, lines));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.use();
    GLCall(glBindVertexArray(VAO));
    GLCall(glDrawArrays(GL_LINES, 0, numLines*2)); // *2 b/c count represents "number of indices to be rendered"
    GLCall(glBindVertexArray(0));
}

void DrawTriangles(RTriangle* triangles, u32 numTriangles)
{
    if (numTriangles == 0) return;
    static Shader shader;
    if (!shader.isValid()) 
    {
        shader = Shader::CreateShaderFromStr(
R"(
layout (location = 0) in vec3 vertPos;
layout (location = 0) in vec4 vertColor;
uniform mat4 mvp;
out vec4 color;
void main(){
    color = vertColor;
	gl_Position = mvp * vec4(vertPos, 1.0);
}
)",
R"(
 
out vec4 FragColor;
in vec4 color;
void main(){
	FragColor = color;
}
)"
        );
    }
    static u32 quadVAO = 0;
    static u32 VBO = 0;
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
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCall(glBindVertexArray(0));
    }
    else 
    {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        //GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RTriangle) * numTriangles, triangles));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.use();
    GLCall(glBindVertexArray(quadVAO));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, numTriangles*3));
    GLCall(glBindVertexArray(0));
}

void RendererDraw()
{
    PROFILE_FUNCTION();
    RendererData& renderer = GetRenderer();
    DrawPoints(renderer.points.get_elements(), renderer.points.size, 10.0f);
    renderer.points.clear();
    DrawLines(renderer.lines.get_elements(), renderer.lines.size);
    renderer.lines.clear();
    DrawTriangles(renderer.triangles.get_elements(), renderer.triangles.size);
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

} // namespace Renderer