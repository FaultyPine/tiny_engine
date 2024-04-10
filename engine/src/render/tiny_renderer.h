#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "math/tiny_math.h"
#include "scene/entity.h"

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstancedBaseVertexBaseInstance.xhtml
// or
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMultiDrawElementsIndirect.xhtml


/*
WIP
Notes:

api thoughts
PushXXXX for client code

PushMesh
PushModel
PushPlane
PushPoint
PushTriangle
PushLine (3d and 2d variants?)
PushCircle (2D)
PushSquare (2D)
PushSphere/Cube/ShapeX
^ corresponding PushSphereWire -- these are all refactors for shapes.cpp

Each of these would also take in the Shader (and optional Material) that they're drawn with.

Should Text (world space and screen space) be a part of this?

Should there be some way to automatically draw things in screen space?


Renderer would store Shaders for depth/other prepasses. Would compile variants for those prepasses.... WHEN? HOW?

PrepForDraw() would be called before the render passes that does some sorting for materials and batches some vertex lists together
DrawToFramebuffer(Framebuffer) would do the actual drawing and flush out all queued draw commands/data

Current forward rendering passes:
Shadow pass for directional/point lights
Depth & Normals pass for main camera
Color pass


-- TODO:
- start with doing PushCircle/PushSquare/PushPlane/all the push methods for simple shapes.
    this should let me get a sense of the structure of things without diving into 
    meshes and models and batching and materials and passes and all that


*/
struct Arena;
struct Model;
struct Shader;
namespace Renderer
{

TAPI void InitializeRenderer(Arena* arena);

TAPI void RendererDraw();

TAPI void PushPoint(const glm::vec3& point, const glm::vec4& color = glm::vec4(1));
TAPI void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1));
TAPI void PushTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
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