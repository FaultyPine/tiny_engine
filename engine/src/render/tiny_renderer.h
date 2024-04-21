#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "math/tiny_math.h"
#include "scene/entity.h"

/*
// new renderer:
// island: 21 draw calls
// sponza: 84 draw calls
// old renderer:

-- TODO:
- allow entities to set rendering flags on themselves like "should I render?" and "enable cast shadows" and "enable receive shadows"
- transfer Shapes2D and Shapes3D funcs to the renderer
- when renderer is fully done, compare against old renderer. (draw calls & actual timings)
*/

struct Arena;
struct Model;
struct Shader;
struct Framebuffer;
namespace Renderer
{
constexpr u32 MAX_NUM_PRIMITIVE_DRAWS = 300;

TAPI void InitializeRenderer(Arena* arena);

TAPI Framebuffer* RendererDraw();

TAPI void PushPoint(const glm::vec3& point, const glm::vec4& color = glm::vec4(1));
TAPI void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1));
TAPI void PushTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
TAPI void PushFrustum(const glm::mat4& projection, const glm::mat4& view, glm::vec4 color = glm::vec4(1));
TAPI void PushModel(const Model& model, const Shader& shader);
TAPI void PushEntity(const EntityRef& entity);

TAPI void PushDebugRenderMarker(const char* name);
TAPI void PopDebugRenderMarker();


// TODO: remove this once renderer is fully integrated
TAPI void EnableInstancing(
    u32 VAO, 
    void* instanceDataBuffer,
    u32 stride, u32 numElements,
    u32& vertexAttributeLocation, 
    u32& instanceVBO);

}

#endif