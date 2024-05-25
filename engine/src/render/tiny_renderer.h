#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "math/tiny_math.h"
#include "scene/entity.h"

/*
-- TODO:
- allow entities to set rendering flags on themselves like "should I render?" and "enable cast shadows" and "enable receive shadows"
- transfer Sprite and Shapes2D and Shapes3D funcs to the renderer
    // still really hate that sprites aren't integrated with the new renderer
    // sprite batching is omega basic stuff that we can absolutely do rn
    // i'd be pretty happy with making our current PushModel (which is really just pushing a mesh + shader combo)
    // a general case for meshes, and our sprites be a special case of that - being we just push a Model with 
    // a single quad mesh
    // FIXME!!!!!

- when renderer is fully done, compare against old renderer. (draw calls & actual timings)
*/

struct Arena;
struct Model;
struct Shader;
struct Framebuffer;
namespace Renderer
{

TAPI void InitializeRenderer(Arena* arena);

TAPI Framebuffer* RendererDraw();

TAPI void PushPoint(const glm::vec3& point, const glm::vec4& color = glm::vec4(1));
TAPI void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1));
TAPI void PushTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
TAPI void PushFrustum(const glm::mat4& projection, const glm::mat4& view, glm::vec4 color = glm::vec4(1));
TAPI void PushModel(const Model& model, const Shader& shader);
TAPI void PushEntity(const EntityRef& entity);

TAPI void SetDebugOutputRenderPass(u32 renderpassIdx);
TAPI const char** GetRenderPassNames(Arena* arena, u32& numNames);

void EnableInstancing(
    u32 VAO, 
    void* instanceDataBuffer,
    u32 stride, u32 numElements,
    u32& vertexAttributeLocation, 
    u32& instanceVBO);

TAPI void PushDebugRenderMarker(const char* name);
TAPI void PopDebugRenderMarker();


} // namespace Renderer

#endif