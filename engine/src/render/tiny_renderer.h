#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "math/tiny_math.h"

struct Vertex 
{
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec3 tangent = glm::vec3(0);
    glm::vec2 texCoords = glm::vec3(0);
    glm::vec3 color = glm::vec3(1);
    inline std::string str() 
    {
        return "[Pos = " + glm::to_string(position) + "  Normals = " + glm::to_string(normal) + " Tangent = " + glm::to_string(tangent) + "  TexCoords = " + glm::to_string(texCoords) + "  Colors = " + glm::to_string(color) + " ]";
    }
};
struct SimpleVertex
{
    glm::vec3 position;
    glm::vec4 color;
};

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
namespace Renderer
{

TAPI void InitializeRenderer(Arena* arena);

TAPI void RendererDraw();

TAPI void PushPoint(const glm::vec3& point, const glm::vec4& color = glm::vec4(1));
TAPI void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1));
TAPI void PushTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);

}

#endif